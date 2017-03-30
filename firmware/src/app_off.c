#include <stdbool.h>

#include "app.h"
#include "cube.h"
#include "main.h"

void app_off(void) {
	cube_disable();
	while(true) {
		wait();
	}
}
