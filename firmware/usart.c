#include "usart.h"

#include <util/setbaud.h>
#include <avr/io.h>

void usart_init() {
	// Set up baud rate
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;
	UCSR0A |= (USE_2X << U2X0);

	// Enable receive and transmit
	UCSR0B = (1 << RXEN0) | (1 << TXEN0);

	// Set frame format to 8N1
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}
