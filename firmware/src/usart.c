#include "usart.h"

#define BAUD 38400

#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/atomic.h>
#include <util/crc16.h>
#include <util/setbaud.h>

#include "cpu.h"
#include "timer.h"

// Port helper macros
#define USART_SEND_BITS ((1 << TXEN0) | (1 << UDRIE0))
#define USART_RECEIVE_BITS ((1 << RXEN0) | (1 << RXCIE0))

#define usart_send_on()	UCSR0B |= USART_SEND_BITS
#define usart_send_off() UCSR0B &= ~USART_SEND_BITS
#define usart_receive_on()	UCSR0B |= USART_RECEIVE_BITS
#define usart_receive_off() UCSR0B &= ~USART_RECEIVE_BITS

#define USART_FRAME_BYTE 0x7E
#define USART_ESCAPE_BYTE 0x7D

typedef struct {
	enum {
		// Some error detected, waiting for the next correct frame boundary
		ERROR,
		// Normal condition, frame boundary received, waiting for frame header
		IDLE,
		// Frame probably starts with an escaped header
		IDLE_ESCAPE,
		// Frame header received, collecting message body bytes
		MESSAGE,
		// Frame header received, collecting message body bytes, the next byte is escaped
		MESSAGE_ESCAPE,
		// The whole message arrived, the next byte must be a frame boundary
		MESSAGE_END
	} state;
	uint8_t read_index;
	uint8_t write_index;
	uint8_t message_length;
	uint8_t message_crc;
} usart_state_t;

usart_message_t input_buffer[USART_INPUT_BUFFER_COUNT] __attribute((section(".noinit")));
usart_state_t input_state __attribute((section(".noinit")));

usart_message_t output_buffer[USART_OUTPUT_BUFFER_COUNT] __attribute((section(".noinit")));
usart_state_t output_state __attribute((section(".noinit")));

#define next_buffer(index, max) ((index) >= (max) - 1 ? 0 : (index) + 1)

// Received data ready interrupt handler
ISR(USART_RX_vect) {
	bool error = (UCSR0A & ((1 << FE0) | (1 << DOR0) | (1 << UPE0))) != 0;
	uint8_t data = UDR0;

	switch(input_state.state) {
		case ERROR:
			if(!error && data == USART_FRAME_BYTE) {
				// Go back to normal state only if a proper frame boundary detected
				input_state.state = IDLE;
			}
			break;
		case IDLE:
			if(error) {
				input_state.state = ERROR;
				break;
			}
			if(data == USART_FRAME_BYTE) {
				// Frame bytes can be repeated any time between frames
				break;
			}
			if(data == USART_ESCAPE_BYTE) {
				// New frame starts immediately with an escape byte
				input_state.state = IDLE_ESCAPE;
				break;
			}
			// New frame starts normally
			goto HEADER;
		case IDLE_ESCAPE:
			if(error || data == USART_ESCAPE_BYTE) {
				// Unexpected escape character or error
				input_state.state = ERROR;
				break;
			}
			if(data == USART_FRAME_BYTE) {
				// Unexpected frame boundary, go back to initial state
				input_state.state = IDLE;
				break;
			}
			// Unescape
			data ^= 0x20;
			// New frame starts
		HEADER:
			*((uint8_t*)&input_buffer[input_state.write_index]) = data;
			input_state.message_crc = _crc8_ccitt_update(0x00, data);
			input_state.message_length = 0;
			input_state.state = MESSAGE;
			break;
		case MESSAGE:
			if(error) {
				input_state.state = ERROR;
				break;
			}
			if(data == USART_FRAME_BYTE) {
				// Frame ended prematurely, go back to initial state
				input_state.state = IDLE;
				break;
			}
			if(data == USART_ESCAPE_BYTE) {
				// Escape sequence starts
				input_state.state = MESSAGE_ESCAPE;
				break;
			}
			// Handle received message body byte
			goto BODY;
		case MESSAGE_ESCAPE:
			if(error || data == USART_ESCAPE_BYTE) {
				// Unexpected escape character or error
				input_state.state = ERROR;
				break;
			}
			if(data == USART_FRAME_BYTE) {
				// Unexpected frame boundary, go back to initial state
				input_state.state = IDLE;
				break;
			}
			// Unescape
			data ^= 0x20;
			// Handle received message body byte
		BODY:
			input_state.message_crc = _crc8_ccitt_update(input_state.message_crc, data);
			if(input_state.message_length == input_buffer[input_state.write_index].length) {
				// Message ended, this last byte was the CRC
				input_state.state = MESSAGE_END;
				break;
			}
			// Append message
			input_buffer[input_state.write_index].body[input_state.message_length++] = data;
			input_state.state = MESSAGE;
			break;
		case MESSAGE_END:
			if(error || data != USART_FRAME_BYTE) {
				// Error or not a frame byte receiver
				input_state.state = ERROR;
				break;
			}
			if(input_state.message_crc == 0x00) {
				// CRC OK, save the frame
				uint8_t next_index = next_buffer(input_state.write_index, USART_INPUT_BUFFER_COUNT);
				if(next_index != input_state.read_index) {
					// We have room for the next frame
					input_state.write_index = next_index;
				}
			}
			// When we get here, we either stored or dropped the frame, but a new frame starts anyways
			input_state.state = IDLE;
			break;
	}
}

// Ready to send data interrupt handler
ISR(USART_UDRE_vect) {
	usart_send_off();
}

void usart_init(void) {
	// Set up baud rate
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;
	UCSR0A |= (USE_2X << U2X0);
	UCSR0B = 0;
	// Set frame format to 8N1
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);

	input_state.state = ERROR;
	input_state.read_index = 0;
	input_state.write_index = 1;

	// Enable receive via interrupts
	// Transmit will be enabled as soon as the first message is sent
	usart_receive_on();
}

void usart_stop(void) {
	usart_receive_off();
	usart_send_off();
}

usart_message_t* usart_receive_message(uint16_t wait_ms) {
	usart_message_t* ret = NULL;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		uint16_t start_time = timer_get_current();
		uint8_t next_index = next_buffer(input_state.read_index, USART_INPUT_BUFFER_COUNT);
		while(next_index == input_state.write_index && !timer_has_elapsed(start_time, wait_ms)) {
			cpu_sleep();
			next_index = next_buffer(input_state.read_index, USART_INPUT_BUFFER_COUNT);
		}
		if(next_index != input_state.write_index) {
			input_state.read_index = next_index;
			ret = &input_buffer[input_state.read_index];
		}
	}
	return ret;
}

