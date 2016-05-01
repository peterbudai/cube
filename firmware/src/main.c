#include <stdbool.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>

#include "cube.h"
#include "led.h"
#include "reset.h"
#include "usart.h"

#define set_bit(x, b, v) (x) = ((x) & ~(1 << (b))) | ((v) << (b))

static void make_frame(uint8_t* frame, uint8_t i, uint8_t m) {
	for(uint8_t l = 0; l < 8; l++) {
		for(uint8_t r = 0; r < 8; r++) {
			for(uint8_t c = 0; c < 8; c++) {
				if(m == 0) {
					set_bit(frame[l * 8 + r], c, (l == i) ? 1 : 0);
				} else if(m == 1) {
					set_bit(frame[l * 8 + r], c, (r == i) ? 1 : 0);
				} else {
					set_bit(frame[l * 8 + r], c, (c == i) ? 1 : 0);
				}
			}
		}
	}
}

int main(void)
{
	// Init output ports
	led_init();
	cube_init();

	// Blink LED
	led_blink(200);

	usart_init();
	cube_enable();

	set_sleep_mode(SLEEP_MODE_IDLE);
	sei();

	uint8_t m = 0;
	uint8_t i = 0;
	bool enabled = true;

	while(true) {
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

		if(enabled && cube_advance_frame(CUBE_FRAME_ASIS)) {
			make_frame(cube_get_frame(), i, m);
			if(++i >= 8) {
				i = 0;
				if(++m >= 3) {
					m = 0;
				}
			}
		}

		sleep_enable();
		sleep_cpu();
		sleep_disable();
	}

	cli();
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);

	cube_disable();
	usart_stop();

	// Blink LED
	led_blink(200);

	if(enabled) {
		perform_reset();
	} else {
		perform_halt();
	}
}
