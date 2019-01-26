#include "app.h"

#include <stdbool.h>

#include "cpu.h"
#include "cube.h"
#include "led.h"
#include "timer.h"

void app_standby(void) {
#ifndef NO_CUBE
	cube_disable();
#endif
#ifndef NO_LED
	for(;;) {
		led_off();
		timer_wait(4750);
		led_on();
		timer_wait(250);
	}
#endif
}
