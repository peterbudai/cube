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

	// Init peripherials and interrup handlers
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
		/*
		uint8_t len = usart_get_received_message_length();
		if(len > 0) {
			uint8_t cmd = usart_get_received_message_byte(0);
			if(cmd == 0x01) {
				usart_send_message_byte(0x81);
			} else if(cmd == 0x02) {
				if(enabled) {
					cube_disable();
					enabled = false;
				} else {
					cube_enable();
					enabled = true;
				}
				uint8_t reply[2] = { 0x82, enabled };
				usart_send_message_buf(reply, 2);
			} else if(cmd == 0x7F) {
				enabled = usart_get_received_message_byte(1);
				break;
			}
			usart_drop_received_message();
		}
		*/
		apps[1]();

		cpu_sleep();
	}

	cli();
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);

	cube_disable();
	usart_stop();

	// Blink LED
	led_blink(200);

	if(enabled) {
		cpu_reset();
	} else {
		cpu_halt();
	}
}
