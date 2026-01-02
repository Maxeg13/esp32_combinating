/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "sdkconfig.h"
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
//#include "driver/gptimer.h"
//#include "esp_timer.h"
//#include "sk6812_led.h"
#include "driver/uart.h"
#include "esp_vfs_dev.h"
//#include "adc.h"
#include "mag_qmc5883l.h"

const static char *TAG = "";

//static ColourState* neutralPtr = new ColourState{20, 20, 20};
//static ColourState* bluePtr = new ColourState{0, 0, 200};
//static ColourState* redPtr = new ColourState{0, 200, 0};

//static bool timer_on_alarm_cb(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx)
//{
////    static const ColourState* targetPtr{neutralPtr};
//
//    skc6812_led_push(targetPtr);
//    targetPtr = targetPtr->targetPtr;
//
//    return true;
//}


extern "C" {
void app_main(void) {
    ////////////////////////////////////////////
    // Set up stdin/stdout to use UART
    setvbuf(stdin, NULL, _IONBF, 0); // No buffering for stdin
    setvbuf(stdout, NULL, _IONBF, 0); // No buffering for stdout

    // Install UART driver for console
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0, 256, 0, 0, NULL, 0));
    // Use the installed UART driver for the console
    esp_vfs_dev_uart_use_driver(UART_NUM_0);

    // console
//    char c[20] = {0};//
//    scanf("%ssu, %ssu, %ssu", &state1Ptr->r, &state1Ptr->g, &state1Ptr->b);
//    printf("\nmy scanf gives: %s\n", c);

//    skc6812_led_Init(GPIO_NUM_32);
//    adc_init();
//
//    // timer
//    gptimer_handle_t gptimer = NULL;
//    gptimer_config_t timer_config = {
//            .clk_src = GPTIMER_CLK_SRC_DEFAULT,
//            .direction = GPTIMER_COUNT_UP,
//            .resolution_hz = 100000,
//    };
//    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));
//
//    gptimer_event_callbacks_t cbs = {
//            .on_alarm = timer_on_alarm_cb,
//    };
//    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, NULL));
//
//    gptimer_alarm_config_t alarm_config = {
//            200000, // period = 2s
//            0, // counter will reload with 0 on alarm event
//            {true}, // enable auto-reload
//    };
//    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config));
//    ESP_ERROR_CHECK(gptimer_enable(gptimer));
    //    ESP_ERROR_CHECK(gptimer_start(gptimer));

    Mag magUp(GPIO_NUM_33, GPIO_NUM_25);
    Mag magDwn(GPIO_NUM_16, GPIO_NUM_17);

    while (true) {
        int16_t x, y, z;
        vTaskDelay(3);
        vTaskDelay(3);

        magUp.read(x, y, z);
        printf("%d,%d,%d,", x, y, z);

        magDwn.read(x, y, z);
        printf("%d,%d,%d\n", x, y, z);
//        if(ctr++ > 6) {
//            ctr = 0;
//            printf("\n");
//        }

//        i2c_master_receive(dev_handle, &rec, 1, 2);
//        printf("data: %d\n", rec);
//        static int i = 0;
//        i++;
//
//        float adc = adc_get();
//        printf("[%5d, %4.3f], ", i, adc_get());
//        if(adc > 1.8) {
//            skc6812_led_push(redPtr);
//        } else if(adc < 1.6) {
//            skc6812_led_push(bluePtr);
//        } else {
//            skc6812_led_push(neutralPtr);
//        }
//        if(i%12 == 0) {
//            printf("\n");
//        }



//        vTaskDelay(pdMS_TO_TICKS(200));
    }
}
}

