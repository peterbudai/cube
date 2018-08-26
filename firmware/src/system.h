#ifndef SYSTEM_H
#define SYSTEM_H

#include <avr/io.h>

#define SYSTEM_TASK 0
#define SYSTEM_STACK_SIZE 96
#define SYSTEM_STACK_ADDR ((void*)RAMEND)
#define SYSTEM_RECV_BUFFER_SIZE 32
#define SYSTEM_SEND_BUFFER_SIZE 64

void system_task_init(void);

void system_run(void);

#endif
