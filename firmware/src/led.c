#include "led.h"

#ifndef NO_LED

#include <avr/io.h>
#include <avr/interrupt.h>

#include "task.h"

// Port numbers and bits
#define LED_PORT PORTD
#define LED_BIT_INDEX PORTD2
#define LED_BIT (1 << LED_BIT_INDEX)

ISR(INT0_vect) {
	task_schedule_unsafe();
}

void led_init(void) {
	// Set output port
	DDRD |= LED_BIT;

	// Led is off by default
	led_off();

	// Set up interrupt on rising edge (led_on)
	EIFR = (1 << INTF0);
	EICRA |= (1 << ISC01) | (1<< ISC00);
	EIMSK |= (1 << INT0);
}

void led_on(void) {
	LED_PORT |= LED_BIT;
}

void led_off(void) {
	LED_PORT &= ~LED_BIT;
}



#endif // NO_LED
