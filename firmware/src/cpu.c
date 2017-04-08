/// @file cpu.c
/// @copyright (C) 2016 Peter Budai
/// Microcontroller CPU state handler routines.

#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/atomic.h>

#include "cpu.h"

void handle_reset(void) {
	// Clear reser flag and disable watchdog
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
