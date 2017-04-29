/**
 * @file draw.h
 * 3D drawing routines.
 *
 * @copyright (C) 2017 Peter Budai
 */

#ifndef DRAW_H
#define DRAW_H

#include <stdbool.h>
#include <stdint.h>

/// Cube directions.
typedef enum {
	/// Direction along the long side of the cube.
	ROWS,
	/// Direction along the short side of the cube.
	COLUMNS,
	/// Direction along the vertical axis.
	LAYERS
} plane_t;

/**
 * Clears the frame completely.
 *
 * @param frame Pointer to the edited frame.
 */
void clear_frame(uint8_t* frame);

/**
 * Sets a single pixel to the given value.
 *
 * @param frame Pointer to the edited frame.
 * @param row Coordinate along the long side of the cube.
 * @param column Coordinate along the short side of the cube.
 * @param layer Coordinate along the vertical dimension of the cube.
 * @param value True to light up the pixel, false to turn it off.
 */
void set_pixel(uint8_t* frame, uint8_t row, uint8_t column, uint8_t layer, bool value);

/**
 * Sets an arbitrary plane at once.
 *
 * @param frame Pointer to the edited frame.
 * @param plane Direction to select a plane along.
 * @param n Index of the plane in the given direction.
 * @param value 8-byte buffer that contains the
 */
void set_plane(uint8_t* frame, plane_t plane, uint8_t n, const uint8_t* value);

#endif
