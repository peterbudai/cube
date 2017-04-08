#include <stdbool.h>

#include "app.h"
#include "cpu.h"
#include "cube.h"

void app_off(void) {
	cube_disable();
	while(true) {
		cpu_sleep();
	}
}
