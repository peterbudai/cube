#include <stdbool.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>

#include "cube.h"
#include "led.h"
#include "reset.h"
#include "usart.h"

#define set_bit(x, b, v) (x) = ((x) & ~(1 << (b))) | ((v) << (b))

void make_frame(uint8_t* frame, uint8_t i, uint8_t m) {
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

	set_sleep_mode(SLEEP_MODE_IDLE);
	cube_enable();
	sei();
	
	uint8_t m = 0;
	while(true) {
		for(uint8_t i = 0; i < 8; i++) {
			while(!cube_advance_frame(CUBE_FRAME_ASIS)) {
				sleep_enable();
				sleep_cpu();
				sleep_disable();
			}
			make_frame(cube_get_frame(), i, m);
		}
		m = (m >= 2) ? 0 : (m + 1);
	}
}
