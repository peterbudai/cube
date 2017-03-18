#ifndef CUBE_H
#define CUBE_H

#include <stdint.h>

#define CUBE_FRAME_SIZE 64
#define CUBE_FRAME_BUFFER_COUNT 16
#define CUBE_FRAME_REPEAT 5
#define CUBE_FRAME_PER_SECOND 25

void cube_init(void);
void cube_enable(void);
void cube_disable(void);

uint8_t cube_get_free_frames(void);
uint8_t* cube_advance_frame(void);

#endif

