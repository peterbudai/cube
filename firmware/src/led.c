#include "led.h"

#include <avr/io.h>

// Port numbers and bits
#define LED_PORT PORTD
#define LED_BIT_INDEX PORTD2
#define LED_BIT (1 << LED_BIT_INDEX)

void led_init(void) {
	// Set output port
	DDRD |= LED_BIT;

	// Led is off by default
	led_off();
}

void led_on(void) {
	LED_PORT |= LED_BIT;
}

void led_off(void) {
	LED_PORT &= ~LED_BIT;
}
