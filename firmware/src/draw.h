#ifndef DRAW_H
#define DRAW_H

#include <stdint.h>

void clear_frame(uint8_t* frame);
void clear_pixel(uint8_t* frame, uint8_t row, uint8_t column, uint8_t layer);
void draw_pixel(uint8_t* frame, uint8_t row, uint8_t column, uint8_t layer);

#endif

