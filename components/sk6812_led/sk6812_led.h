#include "stdint.h"
#include "driver/gpio.h"

/*          add to gpio.h / gpio.c:      */
//  esp_err_t gpio_set_level_insecure(gpio_num_t gpio_num, uint32_t level)
//  {
//      gpio_hal_set_level(gpio_context.gpio_hal, gpio_num, level);
//      return ESP_OK;
// }

struct ColourState {
    static constexpr uint8_t step = 2;

    uint8_t g;
    uint8_t r;
    uint8_t b;

    mutable ColourState* targetPtr = nullptr;
    ///////////////
    void initTarget(const ColourState*) const;
    void print() const;
    void stepTo(const ColourState& targ);
};

void skc6812_led_Init(gpio_num_t num);
void skc6812_led_shine(const ColourState& state);
void skc6812_led_push(const ColourState* state);
void skc6812_led_blue_test();