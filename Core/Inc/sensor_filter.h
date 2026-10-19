#ifndef INC_SENSOR_FILTER_H_
#define INC_SENSOR_FILTER_H_

#include "main.h"

#define FILTER_WINDOW_SIZE 10

typedef struct {
    uint16_t buffer[FILTER_WINDOW_SIZE];
    uint8_t current_index;
    uint32_t sum;
} MovingAverageFilter_t;

void Filter_Init(MovingAverageFilter_t *filter);
uint16_t Filter_Update(MovingAverageFilter_t *filter, uint16_t raw_adc);

#endif /* INC_SENSOR_FILTER_H_ */