#ifndef CUBE_H_
#define CUBE_H_

#include <stdint.h>
#include <stdbool.h>

/**
 * Size of one 3D frame in bytes.
 */
#define CUBE_FRAME_SIZE 64
/**
 * Number of frame buffers.
 */
#define CUBE_FRAME_BUFFER_COUNT 16

#define CUBE_FRAME_ASIS 0
#define CUBE_FRAME_CLEAR 1
#define CUBE_FRAME_COPY 2

/**
 * Initializes the cube ports and interrupts.
 * The cube is turned off by default.
 */
void cube_init();

bool cube_advance_frame(uint8_t method);

uint8_t* cube_get_frame();

/**
 * Turns off the cube and related interrupts.
 */
void cube_disable();

/**
 * Turns on the cube and related interrupts and start processing the frame buffer.
 */
void cube_enable();

#endif /* CUBE_H_ */
