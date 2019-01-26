#pragma once

#include <stdint.h>
#include <stddef.h>

#include "cpu.h"
#include "fifo.h"

/// Task status bits
#define TASK_STOPPED 0x00
#define TASK_SCHEDULED 0x80
#define TASK_WAITING 0x0F
#ifndef NO_CUBE
#define TASK_WAIT_CUBE 0x01
#endif
#ifndef NO_USART
#define TASK_WAIT_RECV 0x02
#define TASK_WAIT_SEND 0x04
#endif
#ifndef NO_TIMER
#define TASK_WAIT_TIMER 0x08
#endif

/// Task descriptor.
typedef struct task {
	uint8_t status;
	void* stack;
	void* stack_start;
	void* stack_end;
#ifndef NO_USART
	fifo_t* recv_fifo;
	fifo_t* send_fifo;
#endif
#ifndef NO_TIMER
	uint16_t wait_until;
#endif
} task_t;

/// Task function prototype.
typedef void (*task_func_t)(void);

/// Number of available task slots.
#define TASK_COUNT 3

/// Parameters of the (always present) idle task
#define IDLE_TASK (TASK_COUNT - 1)
#define IDLE_STACK_START CPU_STACK_START
#define IDLE_STACK_SIZE 64

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
 * The task will get executed as soon as no higher priority tasks are available.
 * @param id Task slot identifier.
 * @param func Function to run.
 */
void task_start(uint8_t id, task_func_t func);

/**
 * Removes a function from a task slot, so it won't be executed anymore.
 * @param id Task slot identifier.
 */
void task_stop(uint8_t id);

/**
 * Stops the current task.
 */
void task_exit(void);

/**
 * Calls the scheduler and yields execution to another, higher priority
 * task, if it is available to run.
 */
void task_yield(void);

/**
 * Starts the task scheduler, and schedules the first available task.
 * It should be normally called from main(), right after the task system
 * was initialized.
 * This function never returns, it becomes the idle task.
 */
void tasks_run(void) __attribute__((noreturn));

/**
 * @name Faster, thread-unsafe functions to be called from other subsytems.
 * Do not call these from task functions.
 */
/// @{

/**
 * Returns the descriptor of the currently executing task.
 * @return Pointer to the task descriptor structure.
 */
task_t* task_current_unsafe(void);

/**
 * Switches to the next available task that is not waiting on any peripherial
 * or the timer subsystem.
 * Interrupts must not be enabled, otherwise the tasks switch will crash.
 */
void task_schedule_unsafe(void);

/// @}
