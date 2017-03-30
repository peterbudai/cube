#include "app.h"

app apps[2] __attribute__((section(".noinit")));

void apps_init(void) {
	apps[0] = app_off;
	apps[1] = app_test;
}

