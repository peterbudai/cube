#include "task.h"

#include <stdlib.h>
#include <avr/io.h>

task_t tasks[TASK_COUNT];
uint8_t current_task;

#define STACK_CANARY ((uint16_t)0x53CA)

/// Stores an return address on the stack.
#define stack_store_addr(ptr, fn) do { \
		*((uint8_t*)(ptr) + 0) = ((uint16_t)(fn)) >> 8; \
		*((uint8_t*)(ptr) + 1) = ((uint16_t)(fn)) & 0xFF; \
	} while(0)

// Stack canary manipulation
#define stack_store_canary(ptr) *((uint16_t*)(ptr)) = STACK_CANARY
#define stack_check_canary(ptr) (*((uint16_t*)(ptr)) == STACK_CANARY)

task_t* task_current_unsafe(void) {
	return &tasks[current_task];
}

void task_init(uint8_t id, void* stack_start, size_t stack_size) {
	// Stack layout after init:
	//
	// Address						Contents
	// -------						--------
	// task.stack_start+1			LO(cpu_reset)
	// task.stack_start				HI(cpu_reset)
	// ... (stack_size-4 bytes) ...
	// task.stack_end+1				HI(STACK_CANARY)
	// task.stack_end				LO(STACK_CANARY)

	tasks[id].status = TASK_STOPPED;
	tasks[id].stack = NULL;
    tasks[id].stack_start = stack_start - 1;
	stack_store_addr(tasks[id].stack_start, cpu_reset);
    tasks[id].stack_end = stack_start - stack_size + 1;
	stack_store_canary(tasks[id].stack_end);
	tasks[id].recv_fifo = NULL;
	tasks[id].send_fifo = NULL;
}

void task_add(uint8_t id, task_func_t func) {
	// Stack layout after adding:
	//
	// Address						Contents
	// -------						--------
	// task.stack_start+1			LO(cpu_reset)
	// task.stack_start				HI(cpu_reset)
	// task.stack+35 				LO(PC)
	// task.stack+34 				HI(PC)
	// task.stack+33 				R31
	// ... (31 bytes) ...	
	// task.stack+02 				R0
	// task.stack+01 				SREG
	// task.stack					(future SP)
	// ...	
	// task.stack_end+1				HI(STACK_CANARY)
	// task.stack_end				LO(STACK_CANARY)

	// Prepare stack
	uint8_t* stack = tasks[id].stack_start - 36;
	// SREG (interrupts disabled)
	stack[1] = 0;
	// R0..R31
	for(uint8_t i = 2; i <= 33; ++i) {
		stack[i] = 0;
	}
	// PC
	stack_store_addr(&stack[34], func);
	tasks[id].stack = stack;

	// Reset FIFOs
	fifo_clear(tasks[id].recv_fifo);
	fifo_clear(tasks[id].send_fifo);

	// Enable
	tasks[id].status = TASK_SCHEDULED;
}

void task_remove(uint8_t id) {
	tasks[id].status = TASK_STOPPED;
	tasks[id].stack = NULL;
}

void tasks_start(void) {
	// Initialize idle task
	// It is initialized only this far (task_add() is not called) on purpose:
	//  - Idle task doesn't need FIFO.
	//  - This setup will result in idle task to take the place of the
	//    caller function (namely main()), which is our goal here.
	task_init(IDLE_TASK, IDLE_STACK_START, IDLE_STACK_SIZE);
	tasks[IDLE_TASK].status = TASK_SCHEDULED;
	current_task = IDLE_TASK;

	task_schedule();
}

__attribute__((noinline, naked)) void task_switch(uint8_t new_task) {
	// Stack layout after saving context:
	//
	// Address		Contents
	// -------		--------
	// SP+35 		LO(PC)
	// SP+34 		HI(PC)
	// SP+33 		R31
	// ...	
	// SP+02 		R0
	// SP+01 		SREG
	// SP+00	

	// Save context
	// PC is saved when calling this function
	asm volatile(
		"push r31 \n\t"
		"push r30 \n\t"
		"push r29 \n\t"
		"push r28 \n\t"
		"push r27 \n\t"
		"push r26 \n\t"
		"push r25 \n\t"
		"push r24 \n\t"
		"push r23 \n\t"
		"push r22 \n\t"
		"push r21 \n\t"
		"push r20 \n\t"
		"push r19 \n\t"
		"push r18 \n\t"
		"push r17 \n\t"
		"push r16 \n\t"
		"push r15 \n\t"
		"push r14 \n\t"
		"push r13 \n\t"
		"push r12 \n\t"
		"push r11 \n\t"
		"push r10 \n\t"
		"push r9 \n\t"
		"push r8 \n\t"
		"push r7 \n\t"
		"push r6 \n\t"
		"push r5 \n\t"
		"push r4 \n\t"
		"push r3 \n\t"
		"push r2 \n\t"
		"push r1 \n\t"
		"push r0 \n\t"
		"in r0, %0 \n\t"
		"push r0 \n\t"
		"" :: "i" _SFR_IO_ADDR(SREG)
	);

	// Switch context
	tasks[current_task].stack = (void*)SP;
	SP = (uint16_t)tasks[new_task].stack;
	current_task = new_task;
	tasks[current_task].stack = NULL;

	// Restore context
	// PC is restored after returning from this function
	asm volatile(
		"pop r0 \n\t"
		"out %0, r0 \n\t"
		"pop r0 \n\t"
		"pop r1 \n\t"
		"pop r2 \n\t"
		"pop r3 \n\t"
		"pop r4 \n\t"
		"pop r5 \n\t"
		"pop r6 \n\t"
		"pop r7 \n\t"
		"pop r8 \n\t"
		"pop r9 \n\t"
		"pop r10 \n\t"
		"pop r11 \n\t"
		"pop r12 \n\t"
		"pop r13 \n\t"
		"pop r14 \n\t"
		"pop r15 \n\t"
		"pop r16 \n\t"
		"pop r17 \n\t"
		"pop r18 \n\t"
		"pop r19 \n\t"
		"pop r20 \n\t"
		"pop r21 \n\t"
		"pop r22 \n\t"
		"pop r23 \n\t"
		"pop r24 \n\t"
		"pop r25 \n\t"
		"pop r26 \n\t"
		"pop r27 \n\t"
		"pop r28 \n\t"
		"pop r29 \n\t"
		"pop r30 \n\t"
		"pop r31 \n\t"
		"ret \n\t"
		"" :: "i" _SFR_IO_ADDR(SREG)
	);
}

void task_schedule(void) {
	uint8_t next_task;
	for(next_task = 0; next_task < TASK_COUNT; ++next_task) {
		if((tasks[next_task].status & TASK_SCHEDULED) && !(tasks[next_task].status & TASK_WAITING)) {
			break;
		}
	}
	
	if(next_task >= TASK_COUNT || !stack_check_canary(tasks[next_task].stack_end)) {
		// Error condition: no runnable task or stack overflow
		cpu_reset();
	}
	if(next_task != current_task) {
		task_switch(next_task);
	}
}
