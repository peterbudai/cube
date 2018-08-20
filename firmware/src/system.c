#include "system.h"

uint8_t system_recv_buffer[SYSTEM_RECV_BUFFER_SIZE] __attribute__((section(".noinit")));
uint8_t system_send_buffer[SYSTEM_SEND_BUFFER_SIZE] __attribute__((section(".noinit")));

void system_run(void) {
}
