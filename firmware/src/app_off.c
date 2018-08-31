#include "app.h"

#include <stdbool.h>

#include "cpu.h"
#include "cube.h"
#include "led.h"
#include "timer.h"

void app_off(void) {
	cube_disable();

	for(bool led = false; true; led = (led ? false : true)) {
		if(led) {
			led_off();
		} else {
			led_on();
		}
		timer_wait(25);
	}
}
