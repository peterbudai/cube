#include "app.h"
#include "fifo.h"

uint8_t app_recv_buffer[APP_RECV_BUFFER_SIZE];
uint8_t app_send_buffer[APP_SEND_BUFFER_SIZE];
fifo_t app_recv_fifo;
fifo_t app_send_fifo;

task_func_t apps[APP_COUNT];

void app_tasks_init(void) {
	// Init USART buffers
    fifo_init(&app_recv_fifo, app_recv_buffer, APP_RECV_BUFFER_SIZE);
	fifo_init(&app_send_fifo, app_send_buffer, APP_SEND_BUFFER_SIZE);

	// Init task descriptor
    task_init(APP_TASK, APP_STACK_START, APP_STACK_SIZE);
	tasks[APP_TASK].recv_fifo = &app_recv_fifo;
    tasks[APP_TASK].send_fifo = &app_send_fifo;

	// Fill applications list
	apps[0] = app_off;
	apps[1] = app_test;
}
