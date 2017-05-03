#include "cpu.h"

#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/atomic.h>

#define STACK_CANARY 0xCC

// Defined by the linker, this is the byte in memory right after all data bytes
extern uint8_t _end;

void handle_reset(void) {
	// Clear reset flag and disable watchdog, so it won't reset again in 15 ms
	MCUSR = 0;
	wdt_disable();
	cli();

	// Put a canary byte right after the data in RAM (the bottom of the stack)
	_end = STACK_CANARY;
}

void cpu_reset(void) {
	// Enable watchdog and wait until it fires
	wdt_enable(WDTO_15MS);
	cpu_halt();
}

void cpu_halt(void) {
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

	// Check if stack canary has ever been overwritten
	if(_end != STACK_CANARY) {
		// Stack overflow, we'd better reset
		cpu_halt();
	}
}
