#include "draw.h"
#include "cube.h"

void clear_frame(uint8_t* frame) {
	for(uint8_t i = 0; i < CUBE_FRAME_SIZE; i++) {
		frame[i] = 0x00;
	}
}

void clear_pixel(uint8_t* frame, uint8_t row, uint8_t column, uint8_t layer) {
	frame[layer * 8 + row] &= ~(1 << column);
}

void draw_pixel(uint8_t* frame, uint8_t row, uint8_t column, uint8_t layer) {
	frame[layer * 8 + row] |= (1 << column);
}

