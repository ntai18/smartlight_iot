#include "sensor_filter.h"

void Filter_Init(MovingAverageFilter_t *filter) {
    filter->current_index = 0;
    filter->sum = 0;
    for (int i = 0; i < FILTER_WINDOW_SIZE; i++) {
        filter->buffer[i] = 0;
    }
}

uint16_t Filter_Update(MovingAverageFilter_t *filter, uint16_t raw_adc) {
    filter->sum -= filter->buffer[filter->current_index];
    filter->buffer[filter->current_index] = raw_adc;
    filter->sum += raw_adc;

    filter->current_index = (filter->current_index + 1) % FILTER_WINDOW_SIZE;

    return (uint16_t)(filter->sum / FILTER_WINDOW_SIZE);
}