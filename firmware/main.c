#include <stdbool.h>
#include <avr/io.h>
#include <util/delay.h>

#include "../shared/led.h"
#include "../shared/reset.h"
#include "../shared/usart.h"

#define ROWH_PORT PORTD
#define ROWH_MASK ((1 << PORTD7) | (1 << PORTD6) | (1 << PORTD5) | (1 << PORTD4))
#define ROWL_PORT PORTC
#define ROWL_MASK ((1 << PORTC3) | (1 << PORTC2) | (1 << PORTC1) | (1 << PORTC0))
#define ENABLE_PORT PORTD
#define ENABLE_BIT (1 << PORTD3)
#define SHIFT_PORT PORTC
#define SHIFT_BIT (1 << PORTC4)
#define STORE_PORT PORTC
#define STORE_BIT (1 << PORTC5)
#define LAYER_PORT PORTB
#define LAYER_MASK ((1 << PORTB2) | (1 << PORTB1) | (1 << PORTB0))

#define enable_off() ENABLE_PORT |= ENABLE_BIT
#define enable_on() ENABLE_PORT &= ~ENABLE_BIT
#define layer_select(l) LAYER_PORT = (LAYER_PORT & ~(LAYER_MASK)) | (l & LAYER_MASK)
#define shift() \
	SHIFT_PORT |= SHIFT_BIT; \
	SHIFT_PORT &= ~(SHIFT_BIT)
#define store() \
	STORE_PORT |= STORE_BIT; \
	STORE_PORT &= ~(STORE_BIT)

void output_frame(uint8_t* frame) {
	for(uint8_t l = 0; l < 8; l++) {
		enable_off();
		layer_select(l);
		for(uint8_t r = 0; r < 8; r++) {
			uint8_t data = frame[l * 8 + r];
			ROWH_PORT = (ROWH_PORT & ~(ROWH_MASK)) | (data & ROWH_MASK);
			ROWL_PORT = (ROWL_PORT & ~(ROWL_MASK)) | (data & ROWL_MASK);
			shift();
		}
		store();
		enable_on();
		_delay_us(1250);
	}
}

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
	DDRB |= LAYER_MASK;
	DDRC |= (ROWL_MASK | SHIFT_BIT | STORE_BIT);
	DDRD |= (ROWH_MASK | LED_BIT | ENABLE_BIT);

	enable_off();

	// Blink LED
	led_blink(200);

	uint8_t buf[64];
	for(uint8_t i = 0; i < 64; i++) {
		buf[i] = 0;
	}

	uint8_t m = 0;
	while(true) {
		for(uint8_t i = 0; i < 8; i++) {
			make_frame(buf, i, m);
			for(uint8_t f = 0; f < 4; f++) {
				output_frame(buf);
			}
		}
		m = (m >= 2) ? 0 : (m + 1);
	}

	reset(RESET_TO_BOOT_CODE);
}
