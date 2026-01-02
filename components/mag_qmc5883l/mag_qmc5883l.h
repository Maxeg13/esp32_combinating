#ifndef ASD
#define ASD

#include "stdint.h"
#include "driver/i2c_master.h"

struct Mag {
    Mag(gpio_num_t sda, gpio_num_t scl);
    void read(int16_t&, int16_t&, int16_t&);

    i2c_master_bus_handle_t bus_handle;
    i2c_master_dev_handle_t dev_handle;
};

#endif