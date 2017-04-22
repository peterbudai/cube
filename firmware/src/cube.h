/**
 * @file cube.h
 * @copyright (C) 2017 Peter Budai
 * LED cube output driver and framebuffer module.
 */

#ifndef CUBE_H
#define CUBE_H

#include <stdint.h>

/**
 * Size in bytes of a single frame.
 * For a 8x8x8 monochrome cube, 8x8=64 bytes are required.
 */
#define CUBE_FRAME_SIZE 64
/**
 * How many frames are available in the framebuffer.
 * One frame is displayed, but the others are available for pre-rendering.
 */
#define CUBE_FRAME_BUFFER_COUNT 16
/**
 * How many times a frame is displayed before moving on to the next one.
 * This gives time for the output image to stabilize visually.
 */
#define CUBE_FRAME_REPEAT 5
/**
 * How many frames are displayed per second.
 * This is the maximum value if frames are rendered fast enough.
 * This gives smooth animations for a human viewer.
 */
#define CUBE_FRAME_PER_SECOND 25

/**
 * Initializes cube output ports and internal state.
 * This does not turn on the cube and the output refresh timer.
 */
void cube_init(void);

/// Turns on the cube by starting the output refresh timer.
void cube_enable(void);

/// Turns off the cube and the output refresh timer.
void cube_disable(void);

/// Returns how many frames are available in the framebuffer for editing.
uint8_t cube_get_free_frames(void);

/**
 * Requests a new frame for editing in the framebuffer.
 * If there is no available frame it waits for at most the given time
 * for one to became available.
 *
 * @param wait_ms Maximum number of milliseconds to wait for a frame
 *     to become available.
 *     Value of 0 will make this function non-blocking.
 *     Value of TIMER_INFINITE will make the the function block indefinitely
 *     until a frame is available.
 * @return Address of the freely editable frame, which is a buffer of
 *     CUBE_FRAME_SIZE bytes.
 *     NULL if no free frame became available during the wait period.
 */
uint8_t* cube_advance_frame(uint16_t wait_ms);

#endif

