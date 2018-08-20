#ifndef SYSTEM_H
#define SYSTEM_H

#include "task.h"

#define SYSTEM_TASK 0
#define SYSTEM_RECV_BUFFER_SIZE 32
#define SYSTEM_SEND_BUFFER_SIZE 64

extern uint8_t system_recv_buffer[];
extern uint8_t system_send_buffer[];

void system_run(void);

#endif
