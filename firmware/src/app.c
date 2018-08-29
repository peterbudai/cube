#include "app.h"

uint8_t app_recv_buffer[APP_RECV_BUFFER_SIZE] __attribute__((section(".noinit")));
uint8_t app_send_buffer[APP_SEND_BUFFER_SIZE] __attribute__((section(".noinit")));

task_func_t apps[APP_COUNT] __attribute__((section(".noinit")));

void app_tasks_init(void) {
	// Init task descriptor
    task_t* task = &tasks[APP_TASK];
    task->stack_start = APP_STACK_ADDR - 1;
    task->stack_end = APP_STACK_ADDR - APP_STACK_SIZE + 1;
    fifo_init(&task->recv_fifo, app_recv_buffer, APP_RECV_BUFFER_SIZE);
    fifo_init(&task->send_fifo, app_send_buffer, APP_SEND_BUFFER_SIZE);

	// Fill applications list
	apps[0] = app_off;
	apps[1] = app_test;
}
