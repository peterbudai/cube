#ifndef _MCU_H_
#define _MCU_H_

#include <stdint.h>

extern volatile uint64_t mcu_ticks;

void mcu_init(int argc, char** argv);
void mcu_start(void);

#endif
