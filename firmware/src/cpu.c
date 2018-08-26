#include "cpu.h"

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/atomic.h>

void cpu_handle_reset(void) {
	// Clear reset flag and disable watchdog, so it won't reset again in 15 ms
	MCUSR = 0;
	wdt_disable();
	cli();
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
