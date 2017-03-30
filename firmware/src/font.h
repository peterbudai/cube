#ifndef FONT_H
#define FONT_H

#include <stdint.h>

#define FONT_CHAR_COUNT 128
#define FONT_CHAR_SIZE 8

void font_load(uint8_t* buf, char chr);

#endif
