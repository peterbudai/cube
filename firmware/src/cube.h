/**
 * @file cube.h
 * @copyright (C) 2017 Peter Budai
 *
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
 * Initializes cube output ports and internal state.
 * This does not turn on the cube and the output refresh timer.
 */
void cube_init(void);

/// Turns on the cube by starting the output refresh timer event processing.
void cube_enable(void);

/// Turns off the cube and stops processing output refresh timer events.
void cube_disable(void);

/**
 * Timer interrupt handler that will periodically refresh cube output
 * to render the frames visually.
 * This will be called by the timer once in every milliseconds, resulting
 * in an approximately 25 Hz frame display rate.
 */
void cube_timer_refresh(void);

/// Returns how many frames are available in the framebuffer for editing.
uint8_t cube_get_free_frames(void);

/**
 * Requests a new frame for editing in the framebuffer.
 * If there is no available frame it waits for at most the given time
 * for one to became available. However, system events are still handled
 * in the meanwhile.
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

