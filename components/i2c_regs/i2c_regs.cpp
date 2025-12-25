#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "driver/i2c_master.h"


#define I2C_MASTER_TIMEOUT_MS       1000

void i2c_reg_write(i2c_master_dev_handle_t handle, uint8_t reg, uint8_t byte) {
    uint8_t cmd[2] = {reg, byte};
    ESP_ERROR_CHECK(i2c_master_transmit(handle, &cmd[0], 2,  I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS));
}

void i2c_reg_read_short(i2c_master_dev_handle_t handle, uint8_t reg, uint16_t& val) {
    uint8_t data[2];
    i2c_master_transmit_receive(handle, &reg, 1, &data[0], 2, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    val = data[0] << 8 | data[1];
}

void i2c_reg_read(i2c_master_dev_handle_t handle, uint8_t reg, uint8_t& val) {
    ESP_ERROR_CHECK(i2c_master_transmit_receive(handle, &reg, 1, &val, 1, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS));
}

void i2c_reg_read(i2c_master_dev_handle_t handle, uint8_t reg, uint8_t* val_ptr, uint8_t size) {
    ESP_ERROR_CHECK(i2c_master_transmit_receive(handle, &reg, 1, val_ptr, size, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS));
}

