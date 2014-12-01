#ifndef PORTS_H_
#define PORTS_H_

#include <avr/io.h>
#include <util/delay.h>

#define LED_PORT PORTD
#define LED_BIT (1 << PORTD2)

#define led_on() LED_PORT |= LED_BIT
#define led_off() LED_PORT &= ~LED_BIT
#define led_toggle() LED_PORT ^= LED_BIT
#define led_blink(ms) \
	led_on(); \
	_delay_ms(ms); \
	led_off()

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

#endif /* PORTS_H_ */
