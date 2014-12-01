#include "reset.h"
#include <stdbool.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

void reset(uint8_t dest) {
	// This register will store the reset destination in case of watchdog reset
	register uint8_t dest_reg asm("r15");
	asm volatile("mov %0, %1" : "=l" (dest_reg) : "r" (dest));

	// Enable watchdog and wait until it fires
	wdt_enable(WDTO_15MS);
	cli();
	while(true);
}
