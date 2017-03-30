#ifndef DRAW_H
#define DRAW_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
	ROWS = 0,
	COLUMNS,
	LAYERS
} plane_t;

void clear_frame(uint8_t* frame);
void set_pixel(uint8_t* frame, uint8_t row, uint8_t column, uint8_t layer, bool value);
void set_plane(uint8_t* frame, plane_t plane, uint8_t n, const uint8_t* value);

#endif

