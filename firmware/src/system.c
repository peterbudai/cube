#include "system.h"

#include <avr/interrupt.h>
#include <avr/sleep.h>

#include "cpu.h"
#include "cube.h"
#include "led.h"
#include "task.h"
#include "timer.h"
#include "usart.h"

uint8_t system_recv_buffer[SYSTEM_RECV_BUFFER_SIZE] __attribute__((section(".noinit")));
uint8_t system_send_buffer[SYSTEM_SEND_BUFFER_SIZE] __attribute__((section(".noinit")));

void system_task_init(void) {
    task_t* task = &tasks[SYSTEM_TASK];
    task->stack_start = SYSTEM_STACK_ADDR - 1;
    task->stack_end = SYSTEM_STACK_ADDR - SYSTEM_STACK_SIZE + 1;
    fifo_init(&task->recv_fifo, system_recv_buffer, SYSTEM_RECV_BUFFER_SIZE);
    fifo_init(&task->send_fifo, system_send_buffer, SYSTEM_SEND_BUFFER_SIZE);
}

void system_run(void) {
	// Init peripherials and interrupt handlers
	led_init();
	cube_init();
	timer_init();
	usart_init();

	// Start blinking status led
	led_blink(200, 2800);

	// Start running background operations
	set_sleep_mode(SLEEP_MODE_IDLE);
	sei();

	while(true);

	// Prepare shutting down
	cli();
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);

	// Disable all peripherials and interrupt sources
	cube_disable();
	usart_stop();
	timer_stop();
	led_off();
}
