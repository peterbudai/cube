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

uint16_t timer_get_current(void) {
	uint16_t value;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		value = timer_value;
	}
	return value;
}

static uint16_t timer_get_elapsed_inner(uint16_t since) {
	return since <= timer_value ? timer_value - since : UINT16_MAX - since + timer_value;
}

uint16_t timer_get_elapsed(uint16_t since) {
	uint16_t elapsed;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		elapsed = timer_get_elapsed_inner(since);
	}
	return elapsed;
}

static bool timer_has_elapsed_inner(uint16_t since, uint16_t ms) {
	return (ms != TIMER_INFINITE) && (timer_get_elapsed_inner(since) >= ms);
}

bool timer_has_elapsed(uint16_t since, uint16_t ms) {
	bool result;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		result = timer_has_elapsed_inner(since, ms);
	}
	return result;
}

static void timer_wait_elapsed_inner(uint16_t since, uint16_t ms) {
	while(!timer_has_elapsed_inner(since, ms)) {
		cpu_sleep();
	}
}

void timer_wait_elapsed(uint16_t since, uint16_t ms) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		timer_wait_elapsed_inner(since, ms);
	}
}

void timer_wait(uint16_t ms) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		timer_wait_elapsed_inner(timer_value, ms);
	}
}
