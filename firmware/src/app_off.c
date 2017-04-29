#include "app.h"

#include <stdbool.h>

#include "cpu.h"
#include "cube.h"
#include "led.h"

void app_off(void) {
	cube_disable();
	led_blink(200, 10000);
	while(true) {
		cpu_sleep();
	}
}
