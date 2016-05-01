#ifndef RESET_H_
#define RESET_H_

/**
 * Code that runs early after reset.
 * Sets watchdog reset logic into default state.
 */
void handle_reset() __attribute__ ((naked)) __attribute__ ((section (".init3")));

/**
 * Properly resets the microcontroller using the watchdog timer.
 */
void perform_reset() __attribute__ ((noreturn));

/**
 * Properly halts the microcontroller.
 */
void perform_halt() __attribute__ ((noreturn));

#endif /* RESET_H_ */
