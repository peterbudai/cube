#ifndef TIMER_H
#define TIMER_H

#include <stdbool.h>
#include <stdint.h>

#define TIMER_INFINITE UINT16_MAX

void timer_init(void);
uint16_t timer_get_current(void);
uint16_t timer_get_elapsed(uint16_t since);
bool timer_has_elapsed(uint16_t since, uint16_t wait_ms);
void timer_wait_elapsed(uint16_t since, uint16_t wait_ms);
void timer_wait(uint16_t ms);

#endif
