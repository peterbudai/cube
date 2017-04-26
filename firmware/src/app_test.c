#include <util/delay.h>
#include <avr/eeprom.h>

#include "app.h"
#include "cube.h"
#include "draw.h"
#include "font.h"
#include "timer.h"
#include "usart.h"

void app_test(void) {
	uint8_t m = 0;
	uint8_t i = 0;
	char c = 'A';
	uint8_t f[FONT_CHAR_SIZE];

	font_load(f, c);
	cube_enable();

	while(true) {
		uint8_t* frame = cube_advance_frame(TIMER_INFINITE);
		clear_frame(frame);
		set_plane(frame, m, i, f);
		if(++i >= 8) {
			i = 0;
			if(++m >= 3) {
				m = 0;
				if(++c == 'Z' + 1) {
					c = 'A';
				}
				font_load(f, c);
			}
		}

		usart_message_t* message = usart_receive_message(250);
		if(message != NULL && message->type == 1 && message->length == 1) {
			c = message->body[0];
			font_load(f, c);
		}
	}
}
