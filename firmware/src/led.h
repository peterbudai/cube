/**
 * @file led.h
 * Manipulation routines for the status LED.
 *
 * @copyright (C) 2017 Peter Budai
 */

#ifndef _LED_H_
#define _LED_H_

#include <stdint.h>

/**
 * Initializes output ports.
 * It does not turn on the led or start blinking.
 */
void led_init(void);

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

#endif
