/**
 * @file cpu.h
 * Microcontroller CPU handler routines.
 *
 * @copyright (C) 2017 Peter Budai
 */

#ifndef CPU_H
#define CPU_H

#include <stdbool.h>

/**
 * Code that runs early after reset.
 * Sets watchdog reset logic into default state.
 */
void cpu_handle_reset(void) __attribute__((naked)) __attribute__((section(".init3")));

/// Properly resets the microcontroller using the watchdog timer.
#define cpu_reset(void) cpu_stop(true)
/// Properly halts the microcontroller.
#define cpu_halt(void) cpu_stop(false)

/**
 * Either halts or resets the microcontroller.
 *
 * @param watchdog Set to true to activate the watchdog timer before halting the CPU.
 *     This will cause the CPU to be reset shortly. Otherwise it will halt completely.
 */
void cpu_stop(bool watchdog) __attribute__((noreturn));

/**
 * Sleeps the microcontroller until an interrupt occurs.
 * This will handle system events as well.
 */
void cpu_sleep(void);

/// Checks for stack owerflow condition and reset if it occured.
void cpu_check_stack(void);

#endif
