#include <stdbool.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>

#include "cube.h"
#include "led.h"
#include "reset.h"
#include "usart.h"

#define set_bit(x, b, v) (x) = ((x) & ~(1 << (b))) | ((v) << (b))

static void make_tree(uint8_t* frame) {
	frame[ 0] = 0b00000000;
	frame[ 1] = 0b00000000;
	frame[ 2] = 0b00010000;
	frame[ 3] = 0b00111000;
	frame[ 4] = 0b00010000;
	frame[ 5] = 0b00000000;
	frame[ 6] = 0b00000000;
	frame[ 7] = 0b00000000;

	frame[ 8] = 0b00010000;
	frame[ 9] = 0b00010000;
	frame[10] = 0b00010000;
	frame[11] = 0b11111110;
	frame[12] = 0b00010000;
	frame[13] = 0b00010000;
	frame[14] = 0b00010000;
	frame[15] = 0b00000000;

	frame[16] = 0b00000000;
	frame[17] = 0b00010000;
	frame[18] = 0b00000000;
	frame[19] = 0b01000100;
	frame[20] = 0b00000000;
	frame[21] = 0b00010000;
	frame[22] = 0b00000000;
	frame[23] = 0b00000000;

	frame[24] = 0b00000000;
	frame[25] = 0b00000000;
	frame[26] = 0b00010000;
	frame[27] = 0b00101000;
	frame[28] = 0b00010000;
	frame[29] = 0b00000000;
	frame[30] = 0b00000000;
	frame[31] = 0b00000000;

	frame[32] = 0b00000000;
	frame[33] = 0b00010000;
	frame[34] = 0b00010000;
	frame[35] = 0b01101100;
	frame[36] = 0b00010000;
	frame[37] = 0b00010000;
	frame[38] = 0b00000000;
	frame[39] = 0b00000000;

	frame[40] = 0b00000000;
	frame[41] = 0b00000000;
	frame[42] = 0b00010000;
	frame[43] = 0b00101000;
	frame[44] = 0b00010000;
	frame[45] = 0b00000000;
	frame[46] = 0b00000000;
	frame[47] = 0b00000000;

	frame[48] = 0b00000000;
	frame[49] = 0b00000000;
	frame[50] = 0b00000000;
	frame[51] = 0b00010000;
	frame[52] = 0b00000000;
	frame[53] = 0b00000000;
	frame[54] = 0b00000000;
	frame[55] = 0b00000000;

	frame[56] = 0b00000000;
	frame[57] = 0b00000000;
	frame[58] = 0b00000000;
	frame[59] = 0b00000000;
	frame[60] = 0b00000000;
	frame[61] = 0b00000000;
	frame[62] = 0b00000000;
	frame[63] = 0b00000000;
}

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

int main()
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
	uint8_t mode = 1;
	
	while(true) {
		uint8_t len = usart_get_received_message_length();
		if(len > 0) {
			uint8_t cmd = usart_get_received_message_byte(0);
			if(cmd == 0x01) {
				usart_send_message_byte(0x81);
			} else if(cmd == 0x02) {
				if(mode == 2) {
					cube_disable();
					mode = 0;
				} else {
					cube_enable();
					mode++;
				}
				uint8_t reply[2] = { 0x82, mode };
				usart_send_message_buf(reply, 2);
			} else if(cmd == 0x7F) {
				mode = usart_get_received_message_byte(1);
				break;
			}
			usart_drop_received_message();
		}
		
		if(mode != 0 && cube_advance_frame(CUBE_FRAME_ASIS)) {
			if(mode == 1) {
				make_frame(cube_get_frame(), i, m);
				if(++i >= 8) {
					i = 0;
					if(++m >= 3) {
						m = 0;
					}
				}
			} else {
				make_tree(cube_get_frame());
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
	
	perform_reset();
}
