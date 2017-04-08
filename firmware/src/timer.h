#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

#define timer_elapsed(then, now) ((then) <= (now) ? (now) - (then) : UINT16_MAX - (then) + (now))

void timer_init(void);
uint16_t timer_get(void);
void timer_wait(uint16_t ms);

#endif
