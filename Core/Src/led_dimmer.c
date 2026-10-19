#include "led_dimmer.h"

// B?ng tra Gamma 2.2 ánh x? t? (0% -> 100%) sang giá tr? PWM (0 -> 999)
static const uint16_t GAMMA_LUT[101] = {
      0,   0,   0,   1,   1,   2,   3,   4,   6,   7,
      9,  11,  14,  16,  19,  23,  26,  30,  34,  38,
     43,  48,  53,  59,  65,  71,  78,  85,  92, 100,
    108, 116, 125, 134, 143, 153, 163, 173, 184, 195,
    206, 218, 230, 243, 256, 269, 283, 297, 311, 326,
    341, 357, 373, 389, 406, 423, 441, 459, 477, 496,
    515, 535, 555, 576, 597, 618, 640, 662, 685, 708,
    732, 756, 780, 805, 831, 856, 883, 909, 937, 964,
    992, 999, 999, 999, 999, 999, 999, 999, 999, 999, // Tinh ch?nh tr?n
    999, 999, 999, 999, 999, 999, 999, 999, 999, 999, 999
}; 
// (Luu ý: B?n có th? tinh ch?nh l?i các ph?n t? cu?i theo công th?c 999 * (i/100.0)^2.2)

void Dimmer_Init(TIM_HandleTypeDef *htim, uint32_t channel) {
    HAL_TIM_PWM_Start(htim, channel);
}

void Dimmer_SetBrightness(TIM_HandleTypeDef *htim, uint32_t channel, uint8_t percent) {
    if (percent > 100) percent = 100;
    uint16_t pwm_pulse = GAMMA_LUT[percent];
    __HAL_TIM_SET_COMPARE(htim, channel, pwm_pulse);
}