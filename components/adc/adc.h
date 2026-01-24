#include "esp_adc/adc_oneshot.h"

void adc_init(adc_channel_t channel);

struct ADC {
    ADC(adc_channel_t channel);
    float get();

    adc_channel_t channel;
    int adc_raw;
    int voltage;
};
