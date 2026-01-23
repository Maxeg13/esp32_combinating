#include "stdint.h"
#include "driver/gpio.h"

/*          add to gpio.h / gpio.c:      */
//  esp_err_t gpio_set_level_insecure(gpio_num_t gpio_num, uint32_t level)
//  {
//      gpio_hal_set_level(gpio_context.gpio_hal, gpio_num, level);
//      return ESP_OK;
// }

struct ColourState {
    static const int steps_n = 8;

    float g;
    float r;
    float b;

    mutable int step_i = 0;

    mutable float g_step = 2;
    mutable float r_step = 2;
    mutable float b_step = 2;

    explicit ColourState(uint8_t g, uint8_t r, uint8_t b):
            g(g), r(r), b(b) {}
//                g((g / g_step) * g_step) ,r((r / step) * step), b((b / step) * step) {};

    mutable ColourState* targetPtr = nullptr;
    ///////////////
    void initTarget(const ColourState*) const;
    void print() const;
    void stepTo(const ColourState& targ);
    void computeStep(const ColourState* targ) const;
};

void skc6812_led_init(gpio_num_t num);
void skc6812_led_shine(const ColourState* state);
void skc6812_led_push(const ColourState* state);
void skc6812_led_blue_test();