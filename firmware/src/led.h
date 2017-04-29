/**
 * @file led.h
 * @copyright (C) 2017 Peter Budai
 *
 * Manipulation routines for the status LED.
 */

#ifndef LED_H
#define LED_H

#include <stdint.h>

void led_init(void);
void led_timer_refresh(void);

void led_on(void);
void led_off(void);
void led_blink(uint16_t period_ms);

#endif

