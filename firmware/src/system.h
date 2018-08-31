#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include "task.h"

#define SYSTEM_TASK 0
#define SYSTEM_STACK_START (IDLE_STACK_START - IDLE_STACK_SIZE)
#define SYSTEM_STACK_SIZE 96
#define SYSTEM_RECV_BUFFER_SIZE 32
#define SYSTEM_SEND_BUFFER_SIZE 64

void system_task_init(void);

void system_run(void);

#endif
