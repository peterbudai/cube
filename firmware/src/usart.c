#include "usart.h"

#define BAUD 38400

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/atomic.h>
#include <util/setbaud.h>

typedef struct {
	uint8_t buffer[USART_BUFFER_SIZE];
	uint8_t write_pos;
	uint8_t read_pos;
} buffer_t;

// 01234567890123456789
// --r=============w--- 14/5
// =======w------r===== 13/6
// b------------------- 0/19
// r==================w 19/0

buffer_t receive_buffer __attribute__((section(".noinit")));
buffer_t send_buffer __attribute__((section(".noinit")));

static inline void buffer_clear(buffer_t* buf) {
	buf->write_pos = 0;
	buf->read_pos = 0;
}

static inline uint8_t buffer_length(buffer_t* buf) {
	if(buf->write_pos >= buf->read_pos) {
		return buf->write_pos - buf->read_pos;
	} else {
		return USART_BUFFER_SIZE - (buf->read_pos - buf->write_pos);
	}
}

static inline uint8_t buffer_available(buffer_t* buf) {
	 return USART_BUFFER_SIZE - buffer_length(buf) - 1;
}

static inline bool buffer_empty(buffer_t* buf) {
	return buffer_length(buf) == 0;
}

static inline bool buffer_full(buffer_t* buf) {
	return buffer_available(buf) == 0;
}

static inline void buffer_put(buffer_t* buf, uint8_t data) {
	buf->buffer[buf->write_pos++] = data;
	if(buf->write_pos >= USART_BUFFER_SIZE) {
		buf->write_pos = 0;
	}
}

static inline uint8_t buffer_get(buffer_t* buf) {
	uint8_t data = buf->buffer[buf->read_pos++];
	if(buf->read_pos >= USART_BUFFER_SIZE) {
		buf->read_pos = 0;
	}
	return data;
}

static inline uint8_t buffer_peek(buffer_t* buf) {
	return buf->buffer[buf->read_pos];
}

static inline uint8_t buffer_pos(buffer_t* buf, uint8_t pos) {
	uint8_t remain = USART_BUFFER_SIZE - buf->read_pos;
	if(remain < pos) {
		return pos - remain;
	} else {
		return buf->read_pos + pos;
	}
}

#define USART_SEND_BITS ((1 << TXEN0) | (1 << UDRIE0))
#define USART_RECEIVE_BITS ((1 << RXEN0) | (1 << RXCIE0))

#define usart_send_on()	UCSR0B |= USART_SEND_BITS
#define usart_send_off() UCSR0B &= ~USART_SEND_BITS
#define usart_receive_on()	UCSR0B |= USART_RECEIVE_BITS
#define usart_receive_off() UCSR0B &= ~USART_RECEIVE_BITS

// Received data ready interrupt handler
ISR(USART_RX_vect) {
	// bool error = if(UCSR0A & ((1 << FE0) | (1 << DOR0) | (1 << UPE0)));
	if(buffer_full(&send_buffer)) {
		usart_receive_off();
	}

	buffer_put(&receive_buffer, UDR0);
}

// Ready to send data interrupt handler
ISR(USART_UDRE_vect) {
	if(buffer_empty(&send_buffer)) {
		usart_send_off();
		return;
	}

	UDR0 = buffer_get(&send_buffer);
}

void usart_init(void) {
	buffer_clear(&receive_buffer);
	buffer_clear(&send_buffer);

	// Set up baud rate
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;
	UCSR0A |= (USE_2X << U2X0);
	UCSR0B = 0;
	// Set frame format to 8N1
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);

	// Enable receive via interrupts (transmit will be enabled when the first message sent)
	usart_receive_on();
}

void usart_stop(void) {
	usart_receive_off();
	usart_send_off();
}

bool usart_send_message_byte(uint8_t data) {
	return usart_send_message_buf(&data, 1);
}

bool usart_send_message_buf(uint8_t* buf, uint8_t length) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		usart_send_on();

		if(buffer_available(&send_buffer) < length + 1) {
			return false;
		}
		buffer_put(&send_buffer, length);
		for(uint8_t i = 0; i < length; ++i) {
			buffer_put(&send_buffer, buf[i]);
		}
	}
	return true;
}

uint8_t usart_get_received_message_length(void) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		uint8_t buf_len = buffer_length(&receive_buffer);
		if(buf_len > 0) {
			uint8_t msg_len = buffer_peek(&receive_buffer);
			if(buf_len > msg_len) {
				return msg_len;
			}
		}
	}
	return 0;
}

uint8_t usart_get_received_message_byte(uint8_t index) {
	uint8_t data;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		data = receive_buffer.buffer[buffer_pos(&receive_buffer, index + 1)];
	}
	return data;
}

void usart_get_received_message_buf(uint8_t index, uint8_t* buf, uint8_t length) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		uint8_t pos = buffer_pos(&receive_buffer, index + 1);
		for(uint8_t i = 0; i < length; ++i) {
			buf[i] = receive_buffer.buffer[pos++];
			if(pos >= USART_BUFFER_SIZE) {
				pos = 0;
			}
		}
	}
}

void usart_drop_received_message(void) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		receive_buffer.read_pos = buffer_pos(&receive_buffer, buffer_peek(&receive_buffer) + 1);
		usart_receive_on();
	}
}
