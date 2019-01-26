#include "system.h"

#include <avr/interrupt.h>
#include <avr/sleep.h>

#include "cpu.h"
#include "cube.h"
#include "led.h"
#include "task.h"
#include "timer.h"
#include "usart.h"

#ifndef NO_USART
uint8_t system_recv_buffer[SYSTEM_RECV_BUFFER_SIZE];
uint8_t system_send_buffer[SYSTEM_SEND_BUFFER_SIZE];
fifo_t system_recv_fifo;
fifo_t system_send_fifo;
#endif

void system_task_init(void) {
	// Init task descriptor
    task_init(SYSTEM_TASK, SYSTEM_STACK_START, SYSTEM_STACK_SIZE);

#ifndef NO_USART
	// Init USART buffers
    fifo_init(&system_recv_fifo, system_recv_buffer, SYSTEM_RECV_BUFFER_SIZE);
    fifo_init(&system_send_fifo, system_send_buffer, SYSTEM_SEND_BUFFER_SIZE);
	tasks[SYSTEM_TASK].recv_fifo = &system_recv_fifo;
	tasks[SYSTEM_TASK].send_fifo = &system_send_fifo;
#endif
}

void system_run(void) {
	// Init peripherials and interrupt handlers
#ifndef NO_LED
	led_init();
#endif
#ifndef NO_CUBE
	cube_init();
#endif
#ifndef NO_TIMER
	timer_init();
#endif
#ifndef NO_USART
	usart_init();
#endif

	set_sleep_mode(SLEEP_MODE_IDLE);
	sei();

	// Start running background operations
	for(;;) {
		timer_wait(1000);
	}

	// Prepare shutting down
	cli();
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);

	// Disable all peripherials and interrupt sources
#ifndef NO_CUBE
	cube_disable();
#endif
#ifndef NO_USART
	usart_stop();
#endif
#ifndef NO_TIMER
	timer_stop();
#endif
#ifndef NO_LED
	led_off();
#endif
}
