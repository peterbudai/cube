#ifndef MCU_H
#define MCU_H

#include <stdint.h>

extern volatile uint64_t mcu_ticks;

void mcu_init(int argc, char** argv);
void mcu_start(void);

#endif
