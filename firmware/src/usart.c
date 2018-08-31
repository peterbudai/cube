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

// Framing constants
#define USART_FRAME_BYTE 0x7E
#define USART_ESCAPE_BYTE 0x7D
#define USART_ESCAPE_MASK 0x20

// Possible states of the receiver
typedef enum {
	// Some error detected, waiting for the next correct frame boundary
	INPUT_ERROR,
	// Normal condition, frame boundary received, waiting for frame header
	INPUT_IDLE,
	// Frame probably starts with an escaped header
	INPUT_IDLE_ESCAPE,
	// Frame header received, collecting message body bytes
	INPUT_MESSAGE,
	// Frame header received, collecting message body bytes, the next byte is escaped
	INPUT_MESSAGE_ESCAPE,
	// The whole message arrived, the next byte must be a frame boundary
	INPUT_FRAME_END
} input_state_t;

// Input message queue
usart_message_t input_buffer[USART_INPUT_BUFFER_COUNT];
// Number of unread messages in the queue
uint8_t input_count;
// Index of the message currently being received
uint8_t input_index;
// Current receiver state
input_state_t input_state;
// Number of body bytes already received
uint8_t input_length;
// Holds the current CRC value of the message bytes already received
uint8_t input_crc;

// Possible states of the transmitter
typedef enum {
	// We are after a frame boundary, ready to send the next message
	OUTPUT_IDLE,
	// The message being transmitted starts with an escaped header
	OUTPUT_IDLE_ESCAPE,
	// The header of the message has been sent and there are still some body bytes to send
	OUTPUT_MESSAGE,
	// The current body byte is escaped
	OUTPUT_MESSAGE_ESCAPE,
	// The full message has been sent, only CRC byte is left to send
	OUTPUT_CRC,
	// The CRC byte is escaped
	OUTPUT_CRC_ESCAPE,
	// The CRC has been sent, a frame boundary has to be sent
	OUTPUT_FRAME_END
} output_state_t;

// Output message queue
usart_message_t output_buffer[USART_OUTPUT_BUFFER_COUNT];
// Number of messages ready to send
uint8_t output_count;
// Index of the message currently being transmitted
uint8_t output_index;
// Current transmitter state
output_state_t output_state;
// Number of body bytes already sent
uint8_t output_length;
// Holds the current CRC value of the message bytes already processed
uint8_t output_crc;

// Received data ready interrupt handler
ISR(USART_RX_vect) {
	bool error = (UCSR0A & ((1 << FE0) | (1 << DOR0) | (1 << UPE0))) != 0;
	uint8_t data = UDR0;

	switch(input_state) {
		case INPUT_ERROR:
			if(!error && data == USART_FRAME_BYTE) {
				// Go back to normal state only if a proper frame boundary detected
				input_state = INPUT_IDLE;
			}
			break;
		case INPUT_IDLE:
			if(error) {
				input_state = INPUT_ERROR;
				break;
			}
			if(data == USART_FRAME_BYTE) {
				// Frame bytes can be repeated any time between frames
				break;
			}
			if(data == USART_ESCAPE_BYTE) {
				// New frame starts immediately with an escape byte
				input_state = INPUT_IDLE_ESCAPE;
				break;
			}
			// New frame starts normally
			goto INPUT_HEADER;
		case INPUT_IDLE_ESCAPE:
			if(error || data == USART_ESCAPE_BYTE) {
				// Unexpected escape character or error
				input_state = INPUT_ERROR;
				break;
			}
			if(data == USART_FRAME_BYTE) {
				// Unexpected frame boundary, go back to initial state
				input_state = INPUT_IDLE;
				break;
			}
			// Unescape
			data ^= USART_ESCAPE_MASK;
			// New frame starts
		INPUT_HEADER:
			input_buffer[input_index].header = data;
			input_crc = _crc8_ccitt_update(0x00, data);
			input_length = 0;
			input_state = INPUT_MESSAGE;
			break;
		case INPUT_MESSAGE:
			if(error) {
				input_state = INPUT_ERROR;
				break;
			}
			if(data == USART_FRAME_BYTE) {
				// Frame ended prematurely, go back to initial state
				input_state = INPUT_IDLE;
				break;
			}
			if(data == USART_ESCAPE_BYTE) {
				// Escape sequence starts
				input_state = INPUT_MESSAGE_ESCAPE;
				break;
			}
			// Handle received message body byte
			goto INPUT_BODY;
		case INPUT_MESSAGE_ESCAPE:
			if(error || data == USART_ESCAPE_BYTE) {
				// Unexpected escape character or error
				input_state = INPUT_ERROR;
				break;
			}
			if(data == USART_FRAME_BYTE) {
				// Unexpected frame boundary, go back to initial state
				input_state = INPUT_IDLE;
				break;
			}
			// Unescape
			data ^= USART_ESCAPE_MASK;
			// Handle received message body byte
		INPUT_BODY:
			input_crc = _crc8_ccitt_update(input_crc, data);
			if(input_length == usart_get_message_length(input_buffer[input_index])) {
				// Message ended, this last byte was the CRC
				input_state = INPUT_FRAME_END;
				break;
			}
			// Append message
			input_buffer[input_index].body[input_length++] = data;
			input_state = INPUT_MESSAGE;
			break;
		case INPUT_FRAME_END:
			if(error || data != USART_FRAME_BYTE) {
				// Error or not a frame byte receiver
				input_state = INPUT_ERROR;
				break;
			}
			if(input_crc == 0x00) {
				// CRC OK, process the frame
				if(input_count < USART_INPUT_BUFFER_COUNT) {
					// We have room for the next frame
					input_index = (input_index < USART_OUTPUT_BUFFER_COUNT - 1) ? input_index + 1 : 0;
					input_count++;
				}
			}
			// When we get here, we either stored or dropped the frame, but a new frame starts anyways
			input_state = INPUT_IDLE;
			break;
	}
}

// Ready to send data interrupt handler
ISR(USART_UDRE_vect) {
	uint8_t data;
	switch(output_state) {
		case OUTPUT_IDLE:
			if(output_count == 0) {
				// We have nothing to send, turn off transmission
				usart_send_off();
				break;
			}
			// Start sending the header
			data = output_buffer[output_index].header;
			output_length = 0;
			output_crc = _crc8_ccitt_update(0x00, data);
			if(data == USART_FRAME_BYTE || data == USART_ESCAPE_BYTE) {
				// Header should be escaped, send escape byte first
				UDR0 = USART_ESCAPE_BYTE;
				output_state = OUTPUT_IDLE_ESCAPE;
				break;
			}
			// Send header byte
			goto OUTPUT_HEADER;
		case OUTPUT_IDLE_ESCAPE:
			// Send the escaped data
			data = output_buffer[output_index].header ^ USART_ESCAPE_MASK;
		OUTPUT_HEADER:
			UDR0 = data;
			if(usart_get_message_length(output_buffer[output_index]) == 0) {
				// Message without body, CRC comes next
				output_state = OUTPUT_CRC;
			} else {
				output_state = OUTPUT_MESSAGE;
			}
			break;
		case OUTPUT_MESSAGE:
			// Send next message body byte
			data = output_buffer[output_index].body[output_length];
			output_crc = _crc8_ccitt_update(output_crc, data);
			if(data == USART_FRAME_BYTE || data == USART_ESCAPE_BYTE) {
				// Body byte should be escaped, send espace byte first
				UDR0 = USART_ESCAPE_BYTE;
				output_state = OUTPUT_MESSAGE_ESCAPE;
				break;
			}
			goto OUTPUT_BODY;
		case OUTPUT_MESSAGE_ESCAPE:
			// Send escaped data byte
			data = output_buffer[output_index].body[output_length] ^ USART_ESCAPE_MASK;
		OUTPUT_BODY:
			UDR0 = data;
			if(++output_length == usart_get_message_length(output_buffer[output_index])) {
				// Whole message was sent, CRC comes next
				output_state = OUTPUT_CRC;
			} else {
				output_state = OUTPUT_MESSAGE;
			}
			break;
		case OUTPUT_CRC:
			data = output_crc;
			if(data == USART_FRAME_BYTE || data == USART_ESCAPE_BYTE) {
				// CRC byte should be escaped, send espace byte first
				UDR0 = USART_ESCAPE_BYTE;
				output_state = OUTPUT_CRC_ESCAPE;
				break;
			}
			// Send CRC byte
			goto OUTPUT_FOOTER;
		case OUTPUT_CRC_ESCAPE:
			// Send escaped CRC byte
			data = output_crc ^ USART_ESCAPE_MASK;
		OUTPUT_FOOTER:
			UDR0 = data;
			// Message is sent, free the buffer
			output_index = (output_index < USART_OUTPUT_BUFFER_COUNT - 1) ? output_index + 1 : 0;
			output_count--;
			output_state = OUTPUT_FRAME_END;
			break;
		case OUTPUT_FRAME_END:
			// Send closing frame byte
			UDR0 = USART_FRAME_BYTE;
			output_state = OUTPUT_IDLE;
			break;
	}
}

void usart_init(void) {
	// Set up baud rate
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;
	UCSR0A |= (USE_2X << U2X0);
	UCSR0B = 0;
	// Set frame format to 8N1
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);

	input_state = INPUT_ERROR;
	input_count = 0;
	input_index = 0;

	output_state = OUTPUT_FRAME_END;
	output_count = 0;
	output_index = 0;

	// Enable receive via interrupts
	// Transmit will be enabled as soon as the first message is sent
	usart_receive_on();
}

void usart_stop(void) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		usart_receive_off();
		usart_send_off();
	}
}

usart_message_t* usart_receive_input_message(uint16_t wait_ms) {
	usart_message_t* ret = NULL;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		uint16_t start_time = timer_get_current_unsafe();
		while(input_count == 0 && !timer_has_elapsed_unsafe(start_time, wait_ms)) {
			cpu_sleep();
		}
		if(input_count > 0) {
			ret = &input_buffer[input_index < input_count ?
				USART_INPUT_BUFFER_COUNT - (input_count - input_index) :
				input_index - input_count];
		}
	}
	return ret;
}

bool usart_drop_input_message(void) {
	bool ret = false;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		if((ret = (input_count > 0))) {
			input_count--;
		}
	}
	return ret;
}

usart_message_t* usart_prepare_output_message(uint16_t wait_ms) {
	usart_message_t* ret = NULL;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		uint16_t start_time = timer_get_current_unsafe();
		while(output_count == USART_OUTPUT_BUFFER_COUNT && !timer_has_elapsed_unsafe(start_time, wait_ms)) {
			cpu_sleep();
		}
		if(output_count < USART_OUTPUT_BUFFER_COUNT) {
			ret = &output_buffer[output_count >= USART_OUTPUT_BUFFER_COUNT - output_index ?
				output_count - (USART_OUTPUT_BUFFER_COUNT - output_index) :
				output_index + output_count];
		}
	}
	return ret;
}

bool usart_send_output_message(void) {
	bool ret = false;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		if((ret = (output_count < USART_OUTPUT_BUFFER_COUNT))) {
			if(output_count++ == 0) {
				// Turn on transmission interrupt, so the output queue will be emptied eventually
				usart_send_on();
			}
		}
	}
	return ret;
}
