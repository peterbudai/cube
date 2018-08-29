#include "cpu.h"

#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/atomic.h>

// The linker defines this pseudo symbol that is located after all global variables.
extern uint8_t _end;

void cpu_init(void) {
	// Set up __zero_reg__: Compiler assumes that R1 must always be zero,
	// so this should be the first thing to do before any C code is executed.
	asm volatile("eor r1, r1 \n\t");
	// Set up SREG: interrupts disabled
	SREG = 0;
	// Set up initial stack just above the data area
	SP = (uint16_t)(&_end + CPU_INIT_STACK_SIZE);
	// Clear reset flag and disable watchdog, so it won't reset again in 15 ms
	MCUSR = 0;
	wdt_disable();
}

void cpu_reset(void) {
	// Enable watchdog so, CPU will be reset soon
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
}
