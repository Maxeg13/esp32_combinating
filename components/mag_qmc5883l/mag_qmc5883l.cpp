#include "mag_qmc5883l.h"
#include "driver/i2c.h"

#include "qmc5883l_regs.h"
#include "i2c_regs.h"
#include "cstring"


Mag::Mag(gpio_num_t sda, gpio_num_t scl) {
    i2c_master_bus_config_t i2c_mst_config = {
            -1,
            sda,
            scl,
            I2C_CLK_SRC_DEFAULT,
            7,
            {true},
            {},
            {1}
    };

    i2c_device_config_t dev_cfg = {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address = 0x0d,
            .scl_speed_hz = 100000,
    };

    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &bus_handle));
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle));

    i2c_reg_write(dev_handle, QMC5883L_RESET,0x01);
    i2c_reg_write(dev_handle,QMC5883L_CONFIG,QMC5883L_CONFIG_OS512|QMC5883L_CONFIG_8GAUSS|QMC5883L_CONFIG_200HZ|QMC5883L_CONFIG_CONT);

}

void Mag::read(int16_t &x, int16_t &y, int16_t &z) {
    uint8_t mas[6] = {};
    i2c_reg_read(dev_handle, QMC5883L_X_LSB, &mas[0], 6);

    memcpy(&x, &mas[0], 2);
    memcpy(&y, &mas[2], 2);
    memcpy(&z, &mas[4], 2);
}