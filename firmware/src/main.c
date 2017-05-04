#include <stdbool.h>
#include <avr/interrupt.h>
#include <avr/signature.h>
#include <avr/sleep.h>

#include "app.h"
#include "cpu.h"
#include "cube.h"
#include "led.h"
#include "timer.h"
#include "system.h"
#include "usart.h"

int main(void)
{
	apps_init();

	// Init peripherials and interrupt handlers
	led_init();
	cube_init();
	timer_init();
	usart_init();

	// Start blinking status led
	led_blink(200, 2800);

	// Start running background operations
	set_sleep_mode(SLEEP_MODE_IDLE);
	sei();

	// Start executing applications
	bool stop_mode = system_run();

	// Prepare shutting down
	cli();
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);

	// Disable all peripherials and interrupt sources
	cube_disable();
	usart_stop();
	timer_stop();
	led_off();

	// Stop the CPU
	cpu_stop(stop_mode);
}
