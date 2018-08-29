/**
 * @file cpu.h
 * Microcontroller CPU handler routines.
 *
 * @copyright (C) 2017 Peter Budai
 */

#ifndef CPU_H
#define CPU_H

/// Size of initial stack, that is used temporary by the setup routines
/// right after boot or reset.
#define CPU_INIT_STACK_SIZE 64

/// Initializes the microcontroller.
/// This is the first code to be executed right after powerup or reset.
void cpu_init(void) __attribute__((naked, section(".init0")));

/// Properly resets the microcontroller using the watchdog timer.
void cpu_reset(void) __attribute__((noreturn));
/// Properly halts the microcontroller.
/// This is the last code to be executed while the CPU is running.
void cpu_halt(void) __attribute__((noreturn, naked, section(".fini0")));

/// Sleeps the microcontroller until an interrupt occurs.
void cpu_sleep(void);

#endif
