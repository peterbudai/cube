#include "system.h"

#include <avr/interrupt.h>
#include <avr/sleep.h>

#include "cpu.h"
#include "cube.h"
#include "led.h"
#include "task.h"
#include "timer.h"
#include "usart.h"

uint8_t system_recv_buffer[SYSTEM_RECV_BUFFER_SIZE];
uint8_t system_send_buffer[SYSTEM_SEND_BUFFER_SIZE];
fifo_t system_recv_fifo;
fifo_t system_send_fifo;

void system_task_init(void) {
	// Init USART buffers
    fifo_init(&system_recv_fifo, system_recv_buffer, SYSTEM_RECV_BUFFER_SIZE);
    fifo_init(&system_send_fifo, system_send_buffer, SYSTEM_SEND_BUFFER_SIZE);

	// Init task descriptor
    task_init(SYSTEM_TASK, SYSTEM_STACK_START, SYSTEM_STACK_SIZE);
	tasks[SYSTEM_TASK].recv_fifo = &system_recv_fifo;
	tasks[SYSTEM_TASK].send_fifo = &system_send_fifo;
}

void system_run(void) {
	cube_init();
	timer_init();

	for(;;) {
		timer_wait(15);
	}

	#if 0
	// Init peripherials and interrupt handlers
	led_init();
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
	#endif
}
