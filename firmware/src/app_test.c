#include "app.h"

#include <stdlib.h>

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

	uint16_t t = timer_get_current();
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

		usart_message_t* message = usart_receive_input_message(125);
		if(message != NULL && usart_get_message_type(*message) == 0x01 && usart_get_message_length(*message) == 1) {
			c = message->body[0];
			font_load(f, c);
			usart_drop_input_message();
		}

		if(timer_has_elapsed(t, 2000) && (message = usart_prepare_output_message(0)) != NULL) {
			usart_set_message_header(*message, 0x9, 1);
			message->body[0] = cube_get_free_frames();
			usart_send_output_message();
			t = timer_get_current();
		}
	}
}
