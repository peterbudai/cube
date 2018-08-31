#include "timer.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#include "cpu.h"
#include "cube.h"
#include "task.h"

/// Continuously incrementing value at each timer tick.
uint16_t timer_value;

/// Timer interrupt handler, called once per millisecond.
ISR(TIMER0_COMPA_vect) {
	// Increase timer and let it overflow
	++timer_value;

	// Drive cube refresh
	bool wake = cube_refresh();

	// Handle tasks waiting for timer
	for(uint8_t i = 0; i < TASK_COUNT; ++i) {
		if((tasks[i].status & TASK_WAIT_TIMER) && tasks[i].wait_until == timer_value) {
			// Remove wait flag if timeout reached
			tasks[i].status &= ~TASK_WAIT_TIMER;
			wake = true;
		}
	}

	if(wake) {
		task_schedule();
	}
}

void timer_init(void)
{
	// Reset timer
	TCNT0 = 0x00;
	// Set interval to 1000 Hz
	OCR0A = F_CPU / 64 / TIMER_FREQ;
	// Set CTC mode
	TCCR0A = (1 << WGM01);
	// Set clock source to F_CPU/64
	TCCR0B = (1 << CS01) | (1 << CS00);
	// Enable timer interrupt
	TIMSK0 = (1 << OCIE0A);

	// Start with zero timer
	timer_value = 0;
}

void timer_stop(void) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		// Disable timer interrupt
		TIMSK0 = 0;
	}
}

uint16_t timer_get_current_unsafe(void) {
	return timer_value;
}

uint16_t timer_get_current(void) {
	uint16_t value;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		value = timer_value;
	}
	return value;
}

uint16_t timer_get_elapsed_unsafe(uint16_t since) {
	return since <= timer_value ? timer_value - since : UINT16_MAX - since + timer_value;
}

uint16_t timer_get_elapsed(uint16_t since) {
	uint16_t elapsed;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		elapsed = timer_get_elapsed_unsafe(since);
	}
	return elapsed;
}

bool timer_has_elapsed_unsafe(uint16_t since, uint16_t ms) {
	return (ms != TIMER_INFINITE) && (timer_get_elapsed_unsafe(since) >= ms);
}

bool timer_has_elapsed(uint16_t since, uint16_t ms) {
	bool result;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		result = timer_has_elapsed_unsafe(since, ms);
	}
	return result;
}

static void timer_wait_elapsed_unsafe(uint16_t since, uint16_t ms) {
	while(!timer_has_elapsed_unsafe(since, ms)) {
		cpu_sleep();
	}
}

void timer_wait_elapsed(uint16_t since, uint16_t ms) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		timer_wait_elapsed_unsafe(since, ms);
	}
}

void timer_wait(uint16_t ms) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		timer_wait_elapsed_unsafe(timer_value, ms);
	}
}
