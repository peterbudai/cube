/// @file cube.h
/// @copyright (C) 2016 Peter Budai
/// Internal interface for the LED cube control peripherial.

#ifndef CUBE_H
#define CUBE_H

#include <stdint.h>

/// Size of one 3D frame in bytes.
#define CUBE_FRAME_SIZE 64
/// Number of frame buffers.
#define CUBE_FRAME_BUFFER_COUNT 16

/// @name Cube frame advancing methods.
/// @{

/// Keep the edited frame as-is.
#define CUBE_FRAME_ASIS 0
/// Clear the edited frame.
#define CUBE_FRAME_CLEAR 1
/// Copy previous frame to the edited one.
#define CUBE_FRAME_COPY 2

/// @}

/// Initializes the cube ports and interrupts.
/// The cube is turned off by default.
void cube_init(void);

/// Close the currently edited frame and advance to the next one.
/// @param method What to do with the next frame.
/// @return The number of frames left (including the currently edited one).
uint8_t cube_advance_frame(uint8_t method);

/// Return the currently edited frame.
/// It is a pointer to consecutive CUBE_FRAME_SIZE bytes.
uint8_t* cube_get_frame(void);

/// Turns off the cube and related interrupts.
void cube_disable(void);

/// Turns on the cube and related interrupts and start processing the frame buffer.
void cube_enable(void);

#endif // CUBE_H

