/**
 * @file led.h
 * Manipulation routines for the status LED.
 *
 * @copyright (C) 2017 Peter Budai
 */

#ifndef LED_H
#define LED_H

#include <stdint.h>

/**
 * Initializes output ports.
 * It does not turn on the led or start blinking.
 */
void led_init(void);

/**
 * Timer interrupt handler that will drive timed led blinking.
 * This will be called by the timer once in every milliseconds.
 */
void led_timer_refresh(void);

/**
 * Turn status led on permanently.
 * This will disable blinking so the led will be on until explicitly turned
 * off again or started to blink.
 */
void led_on(void);

/**
 * Turn status led off permanently.
 * This will disable blinking so the led will be off until explicitly turned
 * on again or started to blink.
 */
void led_off(void);

/**
 * Starts blinking status led with the given period and duty cycle.
 * Blinking always starts by changing led state immediately, so if the led
 * is currently on then this will turn it off for `off_ms` time first, and
 * vice versa.
 *
 * @param on_ms Number of milliseconds to keep the led turned on.
 *     Value @ref TIMER_INFINITE is invalid and causes undefined behavior.
 * @param off_ms Number of milliseconds to keep the led turned off.
 *     Value @ref TIMER_INFINITE is invalid and causes undefined behavior.
 */
void led_blink(uint16_t on_ms, uint16_t off_ms);

#endif
