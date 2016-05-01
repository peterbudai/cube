/// @file led.h
/// @copyright (C) 2016 Peter Budai
/// Manipulation routines for the status LED.

#ifndef LED_H
#define LED_H

#include <avr/io.h>
#include <util/delay.h>

/// @name Port numbers and bits.
/// @{
#define LED_PORT PORTD
#define LED_BIT (1 << PORTD2)
/// @}

/// @name Macros for dealing with output ports.
/// @{
#define led_init() DDRD |= LED_BIT
#define led_on() LED_PORT |= LED_BIT
#define led_off() LED_PORT &= ~LED_BIT
#define led_toggle() LED_PORT ^= LED_BIT
/// @}

/// Convenience macro to blink the status LED for the given interval.
/// This macro uses a busy-wait loop for the timeout.
/// @param ms How long should the LED be lit (milliseconds).
#define led_blink(ms) \
	led_on(); \
	_delay_ms(ms); \
	led_off()

#endif // LED_H

