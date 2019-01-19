/**
 * @file font.h
 * Simple font handling from EEPROM.
 *
 * @copyright (C) 2017 Peter Budai
 */

#ifndef _FONT_H_
#define _FONT_H_

#ifndef NO_CUBE

#include <stdint.h>

/**
 * Number of characters in the font.
 * Full support of 128 ASCII characters.
 */
#define FONT_CHAR_COUNT 128

/**
 * How many bytes a single character occupies.
 * 8x8 bitmap font requires 8 bytes per character.
 */
#define FONT_CHAR_SIZE 8

/**
 * Load single character from the 8x8 font stored in EEPROM.
 *
 * The font is returned scanline by scanline from top to bottom.
 * Within each byte (scanline), the pixels are encoded left to right from MSB to LSB.
 *
 * @param buf Buffer of at least @ref FONT_CHAR_SIZE bytes to hold the loaded character.
 * @param chr Character to load, must fall between 0 and @ref FONT_CHAR_COUNT - 1.
 */
void font_load(uint8_t* buf, char chr);

#endif // NO_CUBE

#endif // _FONT_H_
