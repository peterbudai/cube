/// @file cpu.h
/// @copyright (C) 2017 Peter Budai
/// Interface for the microcontroller CPU handle routines.

#ifndef CPU_H
#define CPU_H

/// Code that runs early after reset.
/// Sets watchdog reset logic into default state.
void handle_reset(void) __attribute__((naked)) __attribute__((section(".init3")));

/// Properly resets the microcontroller using the watchdog timer.
void cpu_reset(void) __attribute__((noreturn));

/// Properly halts the microcontroller.
void cpu_halt(void) __attribute__((noreturn));

/// Sleeps the microcontroller until an interrupt occurs.
void cpu_sleep(void);

#endif // CPU_H

