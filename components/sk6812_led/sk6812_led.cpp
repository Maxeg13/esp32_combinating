#include "freertos/FreeRTOS.h"
#include "sk6812_led.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

static gpio_num_t gpio_num;

#if !defined(CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ_160)
#error "edit NOPS_SLEEP_100NS for actual CPU clocking"
#endif

#define NOPS_SLEEP_100NS    "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" \
                            "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" \
                            "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t"


static QueueHandle_t queue;
static portMUX_TYPE led_spinlock = portMUX_INITIALIZER_UNLOCKED;

static void set_t0h() {
    gpio_set_level_insecure(gpio_num, 1);
}

static void set_t0l() {
    gpio_set_level_insecure(gpio_num, 0);
    asm volatile(
        NOPS_SLEEP_100NS
        NOPS_SLEEP_100NS
        NOPS_SLEEP_100NS
        NOPS_SLEEP_100NS
        NOPS_SLEEP_100NS
        NOPS_SLEEP_100NS
            );
}

static void set_t1h() {
    gpio_set_level_insecure(gpio_num, 1);
    asm volatile(
            NOPS_SLEEP_100NS
            NOPS_SLEEP_100NS
            NOPS_SLEEP_100NS
            );
}

static void set_t1l() {
    gpio_set_level_insecure(gpio_num, 0);
    asm volatile(
            NOPS_SLEEP_100NS
            NOPS_SLEEP_100NS
            NOPS_SLEEP_100NS
            );
}

static void add(float& x, float incr) {
    if(x + incr > 255) x = 255;
    else x+= incr;
}

static void minus(float& x, float decr) {
    if(x - decr < 0) x = 0;
    else x -= decr;
}

void ColourState::print() const{
    printf("(%f, %f, %f)\n", g, r, b);
}

void ColourState::initTarget(const ColourState* p) const {
    targetPtr = const_cast<ColourState*>(p);
}

static void sk6812_led_task(void *pvParameters) {
    queue = xQueueCreate(2, sizeof(ColourState*));

    ColourState* target;
    ColourState state = ColourState{0,0,0};

    while(true) {
        if(xQueueReceive(queue, &target , (TickType_t)0)) {
            state.targetPtr = target;
            state.computeStep(state.targetPtr);
//            state.targetPtr->print();
        }

        if(state.targetPtr != nullptr) {
            state.stepTo(*state.targetPtr);
//            state.print();
        }

        skc6812_led_shine(&state);

        vTaskDelay(pdMS_TO_TICKS(40));
    }
}

////////////////

void ColourState::stepTo(const ColourState &targ) {
    if(step_i > steps_n) return;
    step_i++;

    g += g_step;
    r += r_step;
    b += b_step;
}

void ColourState::computeStep(const ColourState* targ) const {
    step_i = 0;

    g_step = (targ->g - g) / steps_n;
    r_step = (targ->r - r) / steps_n;
    b_step = (targ->b - b) / steps_n;
}

void skc6812_led_shine(const ColourState* state) {
    static uint8_t bits[24]{};
    uint8_t g = state->g;
    uint8_t r = state->r;
    uint8_t b = state->b;
    const uint8_t* colourPtrs[3] = {&g, &r, &b};

    for(int i=0; const auto& colourPtr: colourPtrs) {
        for(int b = 7; (i < sizeof(bits)) && b != -1; i++, b--) {
            bits[i] = (*colourPtr >> b) & 0x1;
        }
    }

    taskENTER_CRITICAL(&led_spinlock);
    for(int i=0; i < sizeof(bits); i++) {
        if(bits[i]) {
            set_t1h();
            set_t1l();
        } else {
            set_t0h();
            set_t0l();
        }
    }

    gpio_set_level_insecure(gpio_num, 0);
    taskEXIT_CRITICAL(&led_spinlock);
}

void skc6812_led_push(const ColourState* state) {
    xQueueSend(queue, &state, (TickType_t)0 );
}

void skc6812_led_init(gpio_num_t num) {
    gpio_num = num;

    gpio_reset_pin(num);
    gpio_set_direction(num, GPIO_MODE_OUTPUT);

    xTaskCreate(sk6812_led_task, "led task", 4096, NULL, 6, NULL);
}

void skc6812_led_blue_test() {
    int i = 0;
    //  green
    for( i=0; i<8; i++) {
//        set_t1h();
//        set_t1l();
        set_t0h();
        set_t0l();
    }

    //  red
    for(; i<16; i++) {
    //    set_t1h();
    //    set_t1l();
        set_t0h();
        set_t0l();
    }

    // blue
    for(; i<24; i++) {
        set_t1h();
        set_t1l();
//        set_t0h();
//        set_t0l();
    }
    set_t0l();
}