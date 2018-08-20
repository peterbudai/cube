#include "app.h"

uint8_t app_recv_buffer[APP_RECV_BUFFER_SIZE] __attribute__((section(".noinit")));
uint8_t app_send_buffer[APP_SEND_BUFFER_SIZE] __attribute__((section(".noinit")));

task_func_t apps[APP_COUNT] __attribute__((section(".noinit")));

void apps_init(void) {
	apps[0] = app_off;
	apps[1] = app_test;
}
