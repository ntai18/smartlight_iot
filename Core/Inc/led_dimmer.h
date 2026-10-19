#ifndef INC_LED_DIMMER_H_
#define INC_LED_DIMMER_H_

#include "main.h"

void Dimmer_Init(TIM_HandleTypeDef *htim, uint32_t channel);
void Dimmer_SetBrightness(TIM_HandleTypeDef *htim, uint32_t channel, uint8_t percent);

#endif /* INC_LED_DIMMER_H_ */
