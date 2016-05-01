#ifndef RESET_H
#define RESET_H

/**
 * Code that runs early after reset.
 * Sets watchdog reset logic into default state.
 */
void handle_reset(void) __attribute__((naked)) __attribute__((section(".init3")));

/**
 * Properly resets the microcontroller using the watchdog timer.
 */
void perform_reset(void) __attribute__((noreturn));

/**
 * Properly halts the microcontroller.
 */
void perform_halt(void) __attribute__((noreturn));

#endif /* RESET_H */
