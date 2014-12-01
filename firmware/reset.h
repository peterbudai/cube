#ifndef RESET_H_
#define RESET_H_

#include <stdint.h>

/**
 * The microcontroller will start executing the application code after the reset.
 */
#define RESET_TO_APP_CODE (0xAC)
/**
 * The microcontroller will start executing the bootloader code after the reset.
 */
#define RESET_TO_BOOT_CODE (0x53)

/**
 * Properly resets the microcontroller using the watchdog timer.
 * @param dest Determines which section will be started after the reset.
 */
void reset(uint8_t dest) __attribute__ ((noreturn));

#endif /* RESET_H_ */
