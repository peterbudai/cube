#include "task.h"

#include <avr/io.h>

#include "app.h"
#include "cpu.h"
#include "system.h"

task_t tasks[2] __attribute__((section(".noinit")));
uint8_t current_task __attribute__((section(".noinit")));

// Stack layout after saving context:
//
// Address	Register
// -------	-------
// SP+35 	PCL
// SP+34 	PCH
// SP+33 	R31
// ...		...
// SP+02 	R0
// SP+01 	SREG

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
	current_task = new_task;
	SP = (uint16_t)tasks[current_task].stack;

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

void task_init(void) {
    tasks[APP_TASK].stack = 0;
    tasks[APP_TASK].status = TASK_STOPPED;
}

void task_schedule(uint8_t n, task_func_t func) {
	uint8_t* stack = ((uint8_t*)RAMEND) - (n * 128) - 35;

	// SREG
	stack[1] = 0;
	// R0..R31
	for(uint8_t i = 2; i <= 33; ++i) {
		stack[i] = 0;
	}
	// PC
	stack[34] = ((uint16_t)func) >> 8;
	stack[35] = ((uint16_t)func) & 0xFF;
	stack[36] = 0x00;
	stack[37] = 0x49;

	tasks[n].stack = stack;
}

void task_handle_timer(void) {
    cpu_check_stack();
    task_switch(current_task == APP_TASK ? SYSTEM_TASK : APP_TASK);
}
