#include "freertos/FreeRTOS.h"
#include "driver/i2c_master.h"

void i2c_reg_write(i2c_master_dev_handle_t handle, uint8_t reg, uint8_t byte);

void i2c_reg_read_short(i2c_master_dev_handle_t handle, uint8_t reg, uint16_t& val);

void i2c_reg_read(i2c_master_dev_handle_t handle, uint8_t reg, uint8_t& val);

void i2c_reg_read(i2c_master_dev_handle_t handle, uint8_t reg, uint8_t* val_ptr, uint8_t size);