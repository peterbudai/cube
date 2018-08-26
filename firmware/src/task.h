#ifndef TASK_H
#define TASK_H

#include <stdint.h>

#include "fifo.h"

/// Task status bits
#define TASK_STOPPED 0x00
#define TASK_SCHEDULED 0x80
#define TASK_WAITING 0x0F
#define TASK_WAIT_CUBE 0x01
#define TASK_WAIT_RECV 0x02
#define TASK_WAIT_SEND 0x04
#define TASK_WAIT_TIMER 0x08

typedef struct {
	uint8_t status;
	void* stack;
	void* stack_start;
	void* stack_end;
	fifo_t recv_fifo;
	fifo_t send_fifo;
} task_t;

typedef void (*task_func_t)(void);

#define TASK_COUNT 2

extern task_t tasks[];

void tasks_init(void);
void task_start(uint8_t id, task_func_t func);
void task_stop(uint8_t id);

void task_schedule(void);
void task_handle_timer(void);

#endif