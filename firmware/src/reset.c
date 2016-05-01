/// @file reset.c
/// @copyright (C) 2016 Peter Budai
/// Microcontroller reset logic.

#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

#include "reset.h"

void handle_reset(void) {
	// Clear reser flag and disable watchdog
	MCUSR = 0;
	wdt_disable();
	cli();
}

void perform_reset(void) {
	// Enable watchdog and wait until it fires
	wdt_enable(WDTO_15MS);
	perform_halt();
}

void perform_halt(void) {
	cli();
	sleep_enable();
	while(true) {
		sleep_cpu();
	}
}

