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

static const float magThrs[2] = {1790, 1835};

static bool isIdle = true;
static uint8_t isIdleMas[10] = {0,0,0,0,0,0,0,0,0,0};
static uint8_t isIdleMasPtr = 0;
static int timeScaling = 150;
static const float magStart = 1640; // 1720
static float magThr = 1798;   // 2240 // 1850
static const float magRange = 170;
static const int fireValMax = 32;
static const float sens = fireValMax/magRange; // 2500
static const float speedSens = sens * 1; // 1.2

struct Contour {
    adc_channel_t channel;
    float magVal;
    float magVal_;
    float magSpeed;
    ADC adc;
    int fireVal = 0;
    gpio_num_t fetPin;
    Contour(adc_channel_t channel, gpio_num_t pin): adc(channel), fetPin(pin) {
        gpio_set_direction(fetPin, GPIO_MODE_OUTPUT);
    }

    void computeMag() {
        magVal_ = magVal;

        // adc
        static float overSamplingRate = 10;
        magVal = 0;
        for(int i = 0; i<overSamplingRate; i++) {
            ets_delay_us(100);
            magVal += adc.get();
        }
        magVal *= 1000/overSamplingRate;

        magVal *= -1;
        magVal += 1700*2 + 400;
    }

    void computeSpeed() {
        magSpeed = magVal - magVal_;
    }

    void computeOutput() {
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
    Contour sideContour{ADC_CHANNEL_7, GPIO_NUM_16};
    ////////////////////////////////////////////
    // Set up stdin/stdout to use UART
//    setvbuf(stdin, NULL, _IONBF, 0); // No buffering for stdin
//    setvbuf(stdout, NULL, _IONBF, 0); // No buffering for stdout

    // Install UART driver for console
//    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0, 256, 0, 0, NULL, 0));
    // Use the installed UART driver for the console
//    esp_vfs_dev_uart_use_driver(UART_NUM_0);

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
//        magDwn.read(x, y, z);

        ets_delay_us(4);

        mainContour.computeMag();
        mainContour.computeSpeed();
        sideContour.computeMag();
        sideContour.computeSpeed();

        mainContour.computeOutput();
        sideContour.fireVal = 16;

        //////////
//        fireVal = fireValMax - 1;
//        isIdle = false;
        //////////
        isIdle = ((mainContour.magVal < magStart) || (mainContour.fireVal == 0));

        isIdleMasPtr++;
        if(isIdleMasPtr >= 10) isIdleMasPtr = 0;
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

//            static float phi = 0;
//            magThr = 1940 + 170 * sin(phi += 0.02);

            bool light = false;
            static bool lightened = false;
            lightened = light;
            for(int i=0; i < 10; i++) light += isIdleMas[i];
            if(light && ! lightened) {
                skc6812_led_push(&idleClr);
            } else {
                skc6812_led_push(&capturedClr);
            }

//            printf("idle: %d, ", isIdle);
//            printf("%d, %d\n", (int)mainContour.magVal, mainContour.fireVal);
            printf("%d, %d\n", (int)mainContour.magVal, (int)sideContour.magVal);
//            printf("%d, %d\n", (int)(mainContour.magVal < magStart), mainContour.fireVal);
        }
    }
}
}
