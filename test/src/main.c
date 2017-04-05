#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "io.h"
#include "mcu.h"
#include "ui.h"

int main(int argc, char** argv) {
	ui_init(&argc, argv);
	leds_init();
	mcu_init(argc, argv);

	mcu_start();
	ui_run();
	return 0;
}
