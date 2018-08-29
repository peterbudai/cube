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

/// Task descriptor.
typedef struct task {
	uint8_t status;
	void* stack;
	void* stack_start;
	void* stack_end;
	fifo_t recv_fifo;
	fifo_t send_fifo;
} task_t;

typedef void (*task_func_t)(void);

/// Number of available task slots.
#define TASK_COUNT 2

/// The task descriptors.
extern task_t tasks[];

/**
 * Initializes the given task slot: stack boundaries and status, but not FIFOs.
 * This must be called before using the task slot. If the task uses network,
 * the FIFOs must be initialized as well.
 * @param id Task slot identifier.
 * @param stack_start Stack start address (the highest address, stack grows downwards from here)
 * @param stack_size Available stack size for this task.
 */
void task_init(uint8_t id, void* stack_start, size_t stack_size);

/**
 * Schedules a function to be run on the given task slot.
 * @param id Task slot identifier.
 * @param func Function to run.
 */
void task_start(uint8_t id, task_func_t func);

/**
 * Removes a function from a task slot.
 * @param id Task slot identifier.
 */
void task_stop(uint8_t id);

void task_schedule(void);
void task_handle_timer(void);

/// Starts the task scheduler and passes the control to it.
void tasks_run(void);

#endif