#include "draw.h"

#ifndef NO_CUBE

#include "cube.h"

void clear_frame(uint8_t* frame) {
	for(uint8_t i = 0; i < CUBE_FRAME_SIZE; i++) {
		frame[i] = 0x00;
	}
}

static void clear_pixel(uint8_t* frame, uint8_t row, uint8_t column, uint8_t layer) {
	frame[layer * 8 + row] &= ~(1 << column);
}

static void draw_pixel(uint8_t* frame, uint8_t row, uint8_t column, uint8_t layer) {
	frame[layer * 8 + row] |= (1 << column);
}

void set_pixel(uint8_t* frame, uint8_t row, uint8_t column, uint8_t layer, bool value) {
	if(value) {
		draw_pixel(frame, row, column, layer);
	} else {
		clear_pixel(frame, row, column, layer);
	}
}

void set_plane(uint8_t* frame, plane_t plane, uint8_t n, const uint8_t* value) {
	switch(plane) {
	case ROWS:
		for(uint8_t l = 0; l < 8; ++l) {
			for(uint8_t c = 0; c < 8; ++c) {
				set_pixel(frame, n, c, l, (value[7 - l] & (1 << c)) != 0);
			}
		}
		break;
	case COLUMNS:
		for(uint8_t l = 0; l < 8; ++l) {
			for(uint8_t r = 0; r < 8; ++r) {
				set_pixel(frame, r, n, l, (value[7 - l] & (1 << r)) != 0);
			}
		}
		break;
	case LAYERS:
		for(uint8_t r = 0; r < 8; ++r) {
			for(uint8_t c = 0; c < 8; ++c) {
				set_pixel(frame, r, c, n, (value[r] & (1 << c)) != 0);
			}
		}
		break;
	}
}

#endif // NO_CUBE
