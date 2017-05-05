#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "io.h"
#include "mcu.h"
#include "uart.h"
#include "ui.h"

int main(int argc, char** argv) {
	io_init();
	ui_init(&argc, argv);
	mcu_init(argc, argv);
	uart_init(argc, argv);

	mcu_start();
	uart_start();
	ui_run();
	return 0;
}
