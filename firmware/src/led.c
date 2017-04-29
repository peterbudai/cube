#include "led.h"

#include <avr/io.h>
#include <util/atomic.h>

#include "timer.h"

// Port numbers and bits
#define LED_PORT PORTD
#define LED_BIT (1 << PORTD2)

// Global variables
uint16_t blink_last;
uint16_t blink_period;

void led_init(void) {
	// Set output port
	DDRD |= LED_BIT;

	// Blinking is disabled by default
	blink_period = TIMER_INFINITE;
}

void led_timer_refresh(void) {
	// If timer is off, return immediately to save CPU resources
	if(blink_period == TIMER_INFINITE) {
		return;
	}

	if(timer_has_elapsed_unsafe(blink_last, blink_period)) {
		// Toggle led and restart blink period
		LED_PORT ^= LED_BIT;
		blink_last = timer_get_current_unsafe();
	}
}

void led_on(void) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		// Turn on led permanently and disable blinking
		LED_PORT |= LED_BIT;
		blink_period = TIMER_INFINITE;
	}
}

void led_off(void) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		// Turn off led permanently and disable blinking
		LED_PORT &= ~LED_BIT;
		blink_period = TIMER_INFINITE;
	}
}

void led_blink(uint16_t period_ms) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		// Start blinking
		blink_period = period_ms;
		blink_last = timer_get_current_unsafe();
	}
}

