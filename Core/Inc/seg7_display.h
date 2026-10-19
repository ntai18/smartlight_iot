#ifndef INC_SEG7_DISPLAY_H_
#define INC_SEG7_DISPLAY_H_

#include "main.h"

// C?u hình chân k?t n?i v?i Module TM1637
#define SEG7_CLK_PORT GPIOB
#define SEG7_CLK_PIN  GPIO_PIN_6
#define SEG7_DIO_PORT GPIOB
#define SEG7_DIO_PIN  GPIO_PIN_7

void Seg7_Init(void);
void Seg7_DisplayPercent(uint8_t percent);

#endif /* INC_SEG7_DISPLAY_H_ */
/* EOF */