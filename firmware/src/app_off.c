#include "app.h"

#include <stdbool.h>

#include "cpu.h"
#include "cube.h"
#include "led.h"

void app_off(void) {
	cube_disable();
	while(true) {
		cpu_sleep();
	}
}
