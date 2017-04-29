#include "led.h"

#include <avr/io.h>
#include <util/atomic.h>

#include "timer.h"

// Port numbers and bits
#define LED_PORT PORTD
#define LED_BIT_INDEX PORTD2
#define LED_BIT (1 << LED_BIT_INDEX)

// Global variables
uint16_t blink_last;
uint16_t blink_period[2];

static void blink_off(void) {
	blink_period[0] = blink_period[1] = TIMER_INFINITE;
}

static void blink_toggle(void) {
	// Toggle led and restart blink period
	LED_PORT ^= LED_BIT;
	blink_last = timer_get_current_unsafe();
}

void led_init(void) {
	// Set output port
	DDRD |= LED_BIT;

	// Blinking is disabled by default
	blink_off();
}

void led_timer_refresh(void) {
	uint16_t period = blink_period[(LED_PORT & LED_BIT) >> LED_BIT_INDEX];

	// If timer is off, return immediately to save CPU resources
	if(period == TIMER_INFINITE) {
		return;
	}

	if(timer_has_elapsed_unsafe(blink_last, period)) {
		blink_toggle();
	}
}

void led_on(void) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		// Turn on led permanently and disable blinking
		LED_PORT |= LED_BIT;
		blink_off();
	}
}

void led_off(void) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		// Turn off led permanently and disable blinking
		LED_PORT &= ~LED_BIT;
		blink_off();
	}
}

void led_blink(uint16_t on_ms, uint16_t off_ms) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		// Start blinking
		blink_period[0] = off_ms;
		blink_period[1] = on_ms;
		blink_toggle();
	}
}

