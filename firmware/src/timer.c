#include "timer.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#include "cpu.h"

// Global variables.
uint16_t timer_value __attribute__((section(".noinit")));

ISR(TIMER1_COMPA_vect) {
	// Increase timer and let it overflow
	timer_value++;
}

void timer_init(void)
{
	// Set interval to 1000 Hz
	OCR1A = F_CPU / 1000;
	// Reset timer
	TCNT1 = 0;
	// Enable timer interrupt
	TIMSK1 = (1 << OCIE1A);
	// Set CTC mode and clock source to F_CPU
	TCCR1A = 0;
	TCCR1B = (1 << WGM12) | (1 << CS10);

	// Start with zero timer
	timer_value = 0;
}

uint16_t timer_get(void) {
	uint16_t value;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		value = timer_value;
	}
	return value;
}

void timer_wait(uint16_t ms) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		uint16_t start = timer_value;
		uint16_t now = timer_value;
		while(timer_elapsed(start, now) < ms) {
			cpu_sleep();
			now = timer_value;
		}
	}
}
