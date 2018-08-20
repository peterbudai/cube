#include "cpu.h"

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/atomic.h>

// Stack canary constants
#define STACK_CANARY_SIZE 4
#define STACK_CANARY_PATTERN 0xCC

// Defined by the linker, this is the byte in memory right after all data bytes
extern uint8_t _end;

void cpu_handle_reset(void) {
	// Clear reset flag and disable watchdog, so it won't reset again in 15 ms
	MCUSR = 0;
	wdt_disable();
	cli();

	// Put canary bytes right after the data in RAM (the bottom of the stack)
	for(uint8_t i = 0; i < STACK_CANARY_SIZE; ++i) {
		*(&_end + i) = STACK_CANARY_PATTERN;
	}
}

void cpu_stop(bool watchdog) {
	// Enable watchdog if requested
	if(watchdog) {
		wdt_enable(WDTO_15MS);
	}

	// Halt the CPU
	cli();
	sleep_enable();
	while(true) {
		// Sleeping with interrupts disabled makes CPU never wake again until
		// hardware or watchdog reset
		sleep_cpu();
	}
}

void cpu_sleep(void) {
	sleep_enable();
	NONATOMIC_BLOCK(NONATOMIC_RESTORESTATE) {
		sleep_cpu();
	}
	sleep_disable();
}

void cpu_check_stack(void) {
	// Check if stack canary has been overwritten
	for(uint8_t i = 0; i < STACK_CANARY_SIZE; ++i) {
		if(*(&_end + i) != STACK_CANARY_PATTERN) {
			// Stack overflow, we'd better reset
			cpu_reset();
		}
	}
}

