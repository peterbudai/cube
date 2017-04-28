#include <setjmp.h>
#include <stdbool.h>
#include <avr/interrupt.h>
#include <avr/signature.h>
#include <avr/sleep.h>

#include "app.h"
#include "cpu.h"
#include "cube.h"
#include "led.h"
#include "timer.h"
#include "usart.h"

//bool job_exit_event;
//jmp_buf job_exit_point;
//	if(job_exit_event) {
//		longjmp(job_exit_point, 1);
//	}

int main(void)
{
	apps_init();

	// Init peripherials and interrupt handlers
	led_init();
	cube_init();
	timer_init();
	usart_init();

	// Blink LED
	led_blink(200);

	// Start running background operations
	bool enabled = true;
	set_sleep_mode(SLEEP_MODE_IDLE);
	sei();

	while(true) {
		apps[1]();
	}

	cli();
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);

	cube_disable();
	usart_stop();
	timer_stop();

	// Blink LED
	led_blink(200);

	if(enabled) {
		cpu_reset();
	} else {
		cpu_halt();
	}
}
