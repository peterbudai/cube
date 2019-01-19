#include "app.h"

#include <stdbool.h>

#include "cpu.h"
#include "cube.h"
#include "led.h"
#include "timer.h"

void app_off(void) {
#ifndef NO_CUBE
	cube_disable();
#endif

	for(bool led = false; true; led = (led ? false : true)) {
#ifndef NO_LED
		if(led) {
			led_off();
		} else {
			led_on();
		}
#endif
		timer_wait(250);
	}
}
