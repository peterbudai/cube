#include "cpu.h"

#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/atomic.h>

void handle_reset(void) {
	// Clear reset flag and disable watchdog, so it won't reset again in 15 ms
	MCUSR = 0;
	wdt_disable();
	cli();
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
}
