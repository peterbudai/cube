#include "app.h"

uint8_t app_recv_buffer[APP_RECV_BUFFER_SIZE] __attribute__((section(".noinit")));
uint8_t app_send_buffer[APP_SEND_BUFFER_SIZE] __attribute__((section(".noinit")));

task_func_t apps[APP_COUNT] __attribute__((section(".noinit")));

void app_tasks_init(void) {
	// Init task descriptor
    task_init(APP_TASK, APP_STACK_START, APP_STACK_SIZE);
    fifo_init(&tasks[APP_TASK].recv_fifo, app_recv_buffer, APP_RECV_BUFFER_SIZE);
    fifo_init(&tasks[APP_TASK].send_fifo, app_send_buffer, APP_SEND_BUFFER_SIZE);

	// Fill applications list
	apps[0] = app_off;
	apps[1] = app_test;
}
