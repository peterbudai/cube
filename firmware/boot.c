#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/boot.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
#include <util/delay.h>

#include "ports.h"
#include "reset.h"
#include "usart.h"

/*
#define SIGRD 5
#define USART_TIMEOUT_TICKS (F_CPU >> 4)

void usart_send(uint8_t data);
uint8_t usart_recv_timeout();

int main()
{
	// Init LED output and blink once
	DDRD |= (1 << DDD2);
	led_blink(200);

	// Init serial and process input
	usart_init();
	while(true) {
		uint8_t buf = usart_recv_timeout();
		if(buf == 'p') {
			// Ping
			usart_send('p');
			usart_send('b');
		} else if(buf == 's') {
			// Device signature
			usart_send('s');
			for(uint8_t addr = 0; addr <= 4; addr += 2) {
				buf = boot_signature_byte_get(addr);
				usart_send(buf);
			}
		} else if(buf == 'd') {
			// Read EEPROM
		} else if(buf == 'r') {
			// Reset
			buf = usart_recv_timeout();
			usart_send('r');
			reset((buf == 'a') ? RESET_TO_APP_CODE : RESET_TO_BOOT_CODE);
		} else {
			// Unknown command
			usart_send('u');
			usart_send(buf);
		}
	}
}

void usart_send(uint8_t data) {
	// Busy-wait until ready to send
	while (!(UCSR0A & (1 << UDRE0)));
	UDR0 = data;
}

uint8_t usart_recv_timeout() {
	uint32_t count = 0;
	while(!(UCSR0A & (1 << RXC0))){
		count++;
		if (!(count & 0x7FFFUL)) {
			led_toggle();
		}
		if (count > USART_TIMEOUT_TICKS) {
			led_off();
			reset(RESET_TO_APP_CODE);
		}
	}
	led_on();
	return UDR0;
}
*/
