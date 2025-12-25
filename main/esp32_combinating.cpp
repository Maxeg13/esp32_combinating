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
#include "i2c_regs.h"
#include "driver/i2c.h"
#include "qmc5883l_mag.h"


i2c_master_bus_config_t i2c_mst_config = {
        -1,
        GPIO_NUM_33,
        GPIO_NUM_25,
        I2C_CLK_SRC_DEFAULT,
        7,
        {true},
        {},
        {1}
};

i2c_master_bus_handle_t bus_handle;

i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x0d,
        .scl_speed_hz = 100000,
};


i2c_master_dev_handle_t dev_handle;

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

    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &bus_handle));
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle));

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

    i2c_reg_write(dev_handle, QMC5883L_RESET,0x01);
    i2c_reg_write(dev_handle,QMC5883L_CONFIG,QMC5883L_CONFIG_OS512|QMC5883L_CONFIG_2GAUSS|QMC5883L_CONFIG_50HZ|QMC5883L_CONFIG_CONT);

    vTaskDelay(100);


    while (true) {
        uint8_t mas[6] = {};
        int16_t x, y, z;
        vTaskDelay(20);

        i2c_reg_read(dev_handle, QMC5883L_X_LSB, &mas[0], 6);

        memcpy(&x, &mas[0], 2);
        memcpy(&y, &mas[2], 2);
        memcpy(&z, &mas[4], 2);

        printf("mas %d %d %d\n", x, y, z);


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

