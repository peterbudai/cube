/**
 * @file cpu.h
 * Microcontroller CPU handler routines.
 *
 * @copyright (C) 2017 Peter Budai
 */

#ifndef _CPU_H_
#define _CPU_H_

#include <avr/io.h>

// The linker defines this pseudo symbol that is located after all global variables.
extern char _end;

/// The address of the end of the stack area in the RAM.
#define CPU_STACK_START ((void*)RAMEND)
#define CPU_STACK_END ((void*)&_end)

/// Initializes the microcontroller.
/// This is the first code to be executed right after powerup or reset
/// and before main() is called.
void cpu_init(void) __attribute__((naked, section(".init3")));

/// Properly resets the microcontroller using the watchdog timer.
void cpu_reset(void) __attribute__((noreturn));
/// Properly halts the microcontroller.
/// This is the last code to be executed while the CPU is running.
void cpu_halt(void) __attribute__((noreturn, naked, section(".fini0")));

/// Sleeps the microcontroller until an interrupt occurs.
void cpu_sleep(void);

#endif
