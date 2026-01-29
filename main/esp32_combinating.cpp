/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "sdkconfig.h"
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gptimer.h"
#include "driver/gpio.h"
#include "rom/ets_sys.h"
//#include "esp_timer.h"
#include "sk6812_led.h"
#include "driver/uart.h"
#include "esp_vfs_dev.h"
#include "adc.h"
//#include "mag_qmc5883l.h"
#include "math.h"

const static char *TAG = "";

static const float magThrs[2] = {1700, 1780};
static bool isIdle = true;
static uint8_t isIdleMas[10] = {0,0,0,0,0,0,0,0,0,0};
static uint8_t isIdleMasPtr = 0;
static int timeScaling = 120;
static const int fireValMax = 35;
static bool turnedOn = false;


struct Contour {
    float magStart = 1640; // 1720
    float magRange = 170;
    float sens = fireValMax/magRange; // 2500
    float speedSens = (fireValMax/magRange) * 1.2; // 1.2
    float speedSenses[3] = {};
    float magThr = 1730;
    adc_channel_t channel;
    float magVal;
    float magVal_;
    float magSpeed;
    ADC adc;
    int fireVal = 0;
    int fireValCtr = 0;
    gpio_num_t fetPin;
    Contour(adc_channel_t channel, gpio_num_t pin): adc(channel), fetPin(pin) {
        gpio_set_direction(fetPin, GPIO_MODE_OUTPUT);
        speedSenses[0] = (fireValMax/magRange) * 0.6;
        speedSenses[1] = (fireValMax/magRange) * 1.2;
        speedSenses[2] = (fireValMax/magRange) * 1.7;
    }

    void calibrateCoord(float x) {
        magThr = 2709 + 4421 * (1/x) + 4713 * (1/(x*x));
    }

    void computeMag() {
        magVal_ = magVal;

        // adc
        static float overSamplingRate = 10;
        magVal = 0;
        for(int i = 0; i<overSamplingRate; i++) {
            ets_delay_us(20);
            magVal += adc.get();
        }
        magVal *= 1000/overSamplingRate;

        magVal *= -1;
        magVal += 1700*2 + 400;

        if(magVal > 1990) {
            speedSens = speedSenses[0];
        } else if(magVal > 1760){
            speedSens = speedSenses[1];
        } else {
//            speedSens = speedSenses[2];
        }
    }

    void computeSpeed() {
        magSpeed = magVal - magVal_;
    }

    virtual void computeOutput() {
        if(magVal < magStart) {
            fireVal = 0;
            return;
        }

        if(magVal < magThr)
            fireVal = fireValMax - magSpeed*speedSens;
        else {
            fireVal = fireValMax - (magVal - magThr) * sens - magSpeed*speedSens;
        }

        if(fireVal < 0 ) fireVal = 0;
        else if(fireVal > fireValMax) {
            fireVal = fireValMax;
        }
    }
};

struct SideContour: public Contour {
    SideContour(adc_channel_t channel, gpio_num_t pin): Contour(channel, pin) {}

//    void computeOutput() override {
//        fireVal = fireValCtr - magSpeed*speedSens*6;
//
//        if(fireVal < 0 ) fireVal = 0;
//        else if(fireVal > fireValMax) {
//            fireVal = fireValMax;
//        }
//    }
};


bool consoleCtrl = 0;

const ColourState idleClr = ColourState{7, 7, 7};
const ColourState capturedClr = ColourState{15, 41, 2};

static bool timer_on_alarm_cb(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx)
{
    consoleCtrl = 1;
    return true;
}

extern "C" {
void app_main(void) {
    Contour mainContour{ADC_CHANNEL_5, GPIO_NUM_19}; // chan 5 is gpio_33
    mainContour.magThr = 1656;

    SideContour sideContour{ADC_CHANNEL_7, GPIO_NUM_16};
    sideContour.magThr = 1570;
    sideContour.speedSens *= 10;
    sideContour.magRange = 28;
    sideContour.magStart = 1602;
    sideContour.sens = fireValMax/sideContour.magRange; // 2500
    ////////////////////////////////////////////
    // Set up stdin/stdout to use UART
    setvbuf(stdin, NULL, _IONBF, 0); // No buffering for stdin
    setvbuf(stdout, NULL, _IONBF, 0); // No buffering for stdout

    // Install UART driver for console
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0, 20, 0, 0, NULL, 0));
    // Use the installed UART driver for the console
    esp_vfs_dev_uart_use_driver(UART_NUM_0);
    esp_vfs_dev_uart_use_nonblocking(UART_NUM_0);

    // console
//    char c[20] = {0};//
//    scanf("%ssu, %ssu, %ssu", &state1Ptr->r, &state1Ptr->g, &state1Ptr->b);
//    printf("\nmy scanf gives: %s\n", c);

    skc6812_led_init(GPIO_NUM_17);

    skc6812_led_push(&idleClr);
//    adc_init();
//
//    // timer
    gptimer_handle_t gptimer = NULL;
    gptimer_config_t timer_config = {
            .clk_src = GPTIMER_CLK_SRC_DEFAULT,
            .direction = GPTIMER_COUNT_UP,
            .resolution_hz = 100000,
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));

    gptimer_event_callbacks_t cbs = {
            .on_alarm = timer_on_alarm_cb,
    };
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, NULL));

    gptimer_alarm_config_t alarm_config = {
            1000, // 10000 for blinking
            0, // counter will reload with 0 on alarm event
            {true}, // enable auto-reload
    };
    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config));
    ESP_ERROR_CHECK(gptimer_enable(gptimer));
    ESP_ERROR_CHECK(gptimer_start(gptimer));

//    printf("hello\n");
//    vTaskDelete(NULL);

//    Mag magUp(GPIO_NUM_33, GPIO_NUM_25);
//    Mag magDwn(GPIO_NUM_16, GPIO_NUM_17);


    while (true) {
        // was magUp and dwn printf("%d,%d,%d,", x, y, z); printf("%d,%d,%d\n", x, y, z);

        ets_delay_us(10);

        mainContour.computeMag();
        mainContour.computeSpeed();

//        sideContour.computeMag();
//        sideContour.computeSpeed();

        if(turnedOn)
            mainContour.computeOutput();
        else mainContour.fireVal = 0;

//        sideContour.computeOutput();
//        sideContour.fireVal = 7;

        //////////
//        fireVal = fireValMax - 1;
//        isIdle = false;
        //////////
        isIdle = ((mainContour.magVal < mainContour.magStart) || (mainContour.fireVal == 0)) ;
//                && ((sideContour.magVal < mainContour.magStart) || (sideContour.fireVal == 0));

//        isIdleMasPtr++;
        ++isIdleMasPtr %= 10;
        isIdleMas[isIdleMasPtr] = isIdle;

        if (!isIdle) {
            gpio_set_level(mainContour.fetPin, 1);
            gpio_set_level(sideContour.fetPin, 1);


            for(int i=0; i<fireValMax; i++) {
                ets_delay_us(timeScaling);
                if( i >= mainContour.fireVal ) {
                    gpio_set_level(mainContour.fetPin, 0);
                }
                if( i >= sideContour.fireVal ) {
                    gpio_set_level(sideContour.fetPin, 0);
                }

            }
            gpio_set_level(mainContour.fetPin, 0);
            gpio_set_level(sideContour.fetPin, 0);
        } else {
            gpio_set_level(mainContour.fetPin, 0);
            gpio_set_level(sideContour.fetPin, 0);
            vTaskDelay(pdMS_TO_TICKS(20));
        }

        if(consoleCtrl) {
            consoleCtrl = 0;

//            static int ctr = 0;
//            ctr++;
//            if(ctr % 100 == 0 ) {
//                static int state = 0;
//                state++;
//                state%=2;
//                magThr = magThrs[state];
//            }

            static float x = -1.6;
            static bool moveDown = false;
            static bool moveUp = false;
            if(turnedOn) {
                x += moveDown * -0.001;
                x += moveUp * 0.001;
                if(x < -1.86) x = -1.86;
                if(x > -1.2) x = 1.2;
                if(moveDown || moveUp)  mainContour.calibrateCoord(x);
            }
//            x += dirDown ? -0.0025 : 0.0025;
//            if(x < -1.86) {
//                dirDown = false;
//            } else if(x > -1.2){
//                dirDown = true;
//            }
//            mainContour.calibrateCoord(x);

            static float sin_;
            static float phi = 0;
//            sin_ = sin(phi += 0.02);
//            static float amp = (magThrs[1] - magThrs[0])/2.;
//            static float middle = (magThrs[1] + magThrs[0])/2.;
//            sideContour.magThr = 1610 + 9 * sin_;
//            mainContour.magThr = middle + amp * sin_;

            bool light = false;
            static bool lightened = false;
            lightened = light;
            for(int i=0; i < 10; i++) light += isIdleMas[i];
            if(light && ! lightened) {
                skc6812_led_push(&idleClr);
            } else {
                skc6812_led_push(&capturedClr);
            }

            static int ctr = 0;
            ctr++;
            ctr %= 5;
            char c;
            if(ctr == 0) {
                volatile int res = scanf("%c", (char *) &c);
                if (res == 1) {
                    switch(c) {
                        case 'w':
                            x += 0.01;
                            moveDown = false;
                            moveUp = false;
                            mainContour.calibrateCoord(x); break;
//                            mainContour.magThr -= 3; break;
                        case 's':
                            x -= 0.01;
                            moveDown = false;
                            moveUp = false;
                            mainContour.calibrateCoord(x); break;
//                            mainContour.magThr += 3; break;
                        case 'a':
                            sideContour.magThr += 2; break;
                        case 'd':
//                            sideContour.magThr -= 2; break;
                            moveUp = false;
                            moveDown ^= true; break;
                        case 'u':
                            moveDown = false;
                            moveUp ^= true; break;
                        case 'c':
                            moveDown = false;
                            moveUp = false;
                            turnedOn ^= true; break;
                            
                    }
//                    printf("x: %f\n", x);
                    printf("mainContour thr: %d, sideContour thr: %d\n", (int)mainContour.magThr, (int)sideContour.magThr);
//                    printf("%d, %d\n", (int)mainContour.magVal, (int)sideContour.magVal);
                }
            }

//            printf("idle: %d, ", isIdle);
//            printf("%d, %d\n", (int)mainContour.magVal, mainContour.fireVal);

//            printf("%d, %d\n", (int)(mainContour.magVal < magStart), mainContour.fireVal);
        }
    }
}
}
