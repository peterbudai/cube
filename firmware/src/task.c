#include "task.h"

#include <stdlib.h>
#include <avr/io.h>

#include "cpu.h"

task_t tasks[TASK_COUNT] __attribute__((section(".noinit")));
uint8_t current_task __attribute__((section(".noinit")));

#define STACK_CANARY ((uint16_t)0x53CA)	// Stored as LE uint16_t: 53CA, stored as RET address: CA53

void tasks_init(void) {
	for(uint8_t i = 0; i < TASK_COUNT; ++i) {
		tasks[i].status = TASK_STOPPED;
		tasks[i].stack = NULL;
		*((uint16_t*)tasks[i].stack_start) = STACK_CANARY;
		*((uint16_t*)tasks[i].stack_end) = STACK_CANARY;
	}
}

// Stack layout after setup:
//
// Address			Register
// -------			--------
// stack_start+1	HI(STACK_CANARY)
// stack_start		LO(STACK_CANARY)
// stack+35 		LO(PC)
// stack+34 		HI(PC)
// stack+33 		R31
// ...
// stack+02 		R0
// stack+01 		SREG
// stack			(future SP)
// ...
// stack_end+1		HI(STACK_CANARY)
// stack_end		LO(STACK_CANARY)

void task_start(uint8_t id, task_func_t func) {
	// Prepare stack
	uint8_t* stack = tasks[id].stack_start - 36;
	// SREG (interrupts disabled)
	stack[1] = 0;
	// R0..R31
	for(uint8_t i = 2; i <= 33; ++i) {
		stack[i] = 0;
	}
	// PC
	stack[34] = ((uint16_t)func) >> 8;
	stack[35] = ((uint16_t)func) & 0xFF;
	tasks[id].stack = stack;

	// Reset FIFOs
	tasks[id].recv_fifo.start = 0;
	tasks[id].recv_fifo.size = 0;

	tasks[id].send_fifo.start = 0;
	tasks[id].send_fifo.size = 0;

	// Enable
	tasks[id].status = TASK_SCHEDULED;
}

void task_stop(uint8_t id) {
	tasks[id].status = TASK_STOPPED;
	tasks[id].stack = NULL;
}

// Stack layout after saving context:
//
// Address	Register
// -------	--------
// SP+35 	LO(PC)
// SP+34 	HI(PC)
// SP+33 	R31
// ...
// SP+02 	R0
// SP+01 	SREG
// SP+00

__attribute__((noinline)) void task_switch(uint8_t new_task) {
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
		"" :: "i" _SFR_IO_ADDR(SREG)
	);
}

void task_check_stack(uint8_t num) {
	// Check if stack canary has been overwritten
	if(*((uint16_t*)tasks[num].stack_start) != STACK_CANARY || *((uint16_t*)tasks[num].stack_start) != STACK_CANARY) {
		// Stack overflow, we'd better reset
		cpu_reset();
	}
}

void task_schedule(void) {
	uint8_t next_task;
	for(next_task = 0; next_task < TASK_COUNT; ++next_task) {
		if((tasks[next_task].status & TASK_SCHEDULED != 0) && (tasks[next_task].status & TASK_WAITING == 0)) {
			break;
		}
	}
	if(next_task < TASK_COUNT) {
		task_check_stack(next_task);
		if(next_task != current_task) {
			task_switch(next_task);
		}
	}
}

void task_handle_timer(void) {
	task_schedule();
}
