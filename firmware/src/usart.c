#include "usart.h"

#ifndef NO_USART

#define BAUD 38400

#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/atomic.h>
#include <util/crc16.h>
#include <util/setbaud.h>

#include "cpu.h"
#include "fifo.h"
#include "task.h"
#include "timer.h"

// Framing constants
#define USART_FRAME_BYTE 0x7E
#define USART_ESCAPE_BYTE 0x7D
#define USART_ESCAPE_MASK 0x20

// Framing helper marcros
#define USART_ADDRESS_BITS 1
#define USART_LENGTH_BITS (8 - USART_ADDRESS_BITS)
#define USART_LENGTH_MAX ((1 << USART_LENGTH_BITS) - 1)
#define usart_get_message_address(header) ((header) >> USART_LENGTH_BITS)
#define usart_get_message_length(header) ((header) & USART_LENGTH_MAX)
#define usart_get_message_header(address, length) (((address) << USART_LENGTH_BITS) | ((length) & USART_LENGTH_MAX))

#ifndef NO_USART_RECV

// Port helper macros
#define USART_RECEIVE_BITS ((1 << RXEN0) | (1 << RXCIE0))
#define usart_receive_on()	UCSR0B |= USART_RECEIVE_BITS
#define usart_receive_off() UCSR0B &= ~USART_RECEIVE_BITS

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

// Current receiver state
input_state_t input_state;
// Destination task for the currently received bytes
uint8_t input_task;
// Number of body bytes still left to be received
uint8_t input_length;
// Holds the current CRC value of the message bytes already received
uint8_t input_crc;

// Received data ready interrupt handler
ISR(USART_RX_vect) {
	bool wake = false;
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
			input_task = usart_get_message_address(data);
			if(tasks[input_task].recv_fifo == NULL) {
				// If the receiver task does not accept data, drop the frame
				input_state = INPUT_ERROR;
				break;
			}
			input_length = usart_get_message_length(data);
			if(!fifo_begin_push(tasks[input_task].recv_fifo, input_length)) {
				// If the receiver buffer is full, drop the frame
				input_state = INPUT_ERROR;
				break;
			}
			input_crc = _crc8_ccitt_update(0x00, data);
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
			if(input_length == 0) {
				// Message ended, this last byte was the CRC
				input_state = INPUT_FRAME_END;
				break;
			}
			// Append message
			fifo_push(tasks[input_task].recv_fifo, data);
			input_length--;
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
				fifo_commit_push(tasks[input_task].recv_fifo);
				// Wake up task if it is waiting for receive
				if(tasks[input_task].status & TASK_WAIT_RECV) {
					tasks[input_task].status &= ~TASK_WAITING;
					wake = true;
				}
			}
			// When we get here, we either stored or dropped the frame, but a new frame starts anyways
			input_state = INPUT_IDLE;
			break;
	}

	if(wake) {
		task_schedule_unsafe();
	}
}

#endif // NO_USART_RECV

#ifndef NO_USART_SEND

// Port helper macros
#define USART_SEND_BITS ((1 << TXEN0) | (1 << UDRIE0))
#define usart_send_on()	UCSR0B |= USART_SEND_BITS
#define usart_send_off() UCSR0B &= ~USART_SEND_BITS

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

// Current transmitter state
output_state_t output_state;
// Task buffer for the currently sent bytes
uint8_t output_task;
// Number of body bytes still left to be sent
uint8_t output_length;
// Holds the current CRC value of the message bytes already processed
uint8_t output_crc;

// Ready to send data interrupt handler
ISR(USART_UDRE_vect) {
	bool wake = false;
	uint8_t data;
	switch(output_state) {
		case OUTPUT_IDLE:
			// Look for a task that has data to send
			for(output_task = 0; output_task < TASK_COUNT; ++output_task) {
				if(tasks[output_task].send_fifo != NULL && tasks[output_task].send_fifo->size > 0) {
					break;
				}
			}
			if(output_task >= TASK_COUNT) {
				// We have nothing to send, turn off transmission
				usart_send_off();
				break;
			}

			// Start sending the header
			output_length = tasks[output_task].send_fifo->size & USART_LENGTH_MAX;
			fifo_begin_pop(tasks[output_task].send_fifo, output_length);
			data = usart_get_message_header(output_task, output_length);
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
			data = usart_get_message_header(output_task, output_length) ^ USART_ESCAPE_MASK;
		OUTPUT_HEADER:
			UDR0 = data;
			if(output_length == 0) {
				// Message without body, CRC comes next
				output_state = OUTPUT_CRC;
			} else {
				output_state = OUTPUT_MESSAGE;
			}
			break;
		case OUTPUT_MESSAGE:
			// Send next message body byte
			data = fifo_peek(tasks[output_task].send_fifo);
			output_crc = _crc8_ccitt_update(output_crc, data);
			if(data == USART_FRAME_BYTE || data == USART_ESCAPE_BYTE) {
				// Body byte should be escaped, send espace byte first
				UDR0 = USART_ESCAPE_BYTE;
				output_state = OUTPUT_MESSAGE_ESCAPE;
				break;
			}
			fifo_pop(tasks[output_task].send_fifo);
			goto OUTPUT_BODY;
		case OUTPUT_MESSAGE_ESCAPE:
			// Send escaped data byte
			data = fifo_pop(tasks[output_task].send_fifo) ^ USART_ESCAPE_MASK;
		OUTPUT_BODY:
			UDR0 = data;
			if(--output_length == 0) {
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
			fifo_commit_pop(tasks[output_task].send_fifo);
			// Wake up task if it is waiting to send
			if(tasks[output_task].status & TASK_WAIT_SEND) {
				tasks[output_task].status &= ~TASK_WAITING;
				wake = true;
			}
			output_state = OUTPUT_FRAME_END;
			break;
		case OUTPUT_FRAME_END:
			// Send closing frame byte
			UDR0 = USART_FRAME_BYTE;
			output_state = OUTPUT_IDLE;
			break;
	}

	// Handle possible task switch
	if(wake) {
		task_schedule_unsafe();
	}
}

#endif // NO_USART_SEND

void usart_init(void) {
	// Set up baud rate
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;
	UCSR0A |= (USE_2X << U2X0);
	UCSR0B = 0;
	// Set frame format to 8N1
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);

#ifndef NO_USART_SEND
	output_state = OUTPUT_FRAME_END;
	output_task = TASK_COUNT;
	output_length = 0;
#endif

#ifndef NO_USART_RECV
	// Init state machines
	input_state = INPUT_ERROR;
	input_task = TASK_COUNT;
	input_length = 0;

	// Enable receive via interrupts
	// Transmit will be enabled as soon as the first message is sent
	usart_receive_on();
#endif
}

void usart_stop(void) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
#ifndef NO_USART_RECV
		usart_receive_off();
#endif
#ifndef NO_USART_SEND
		usart_send_off();
#endif
	}
}

#ifndef NO_USART_RECV
bool usart_receive_bytes(uint8_t* dest, size_t count, uint16_t wait_ms) {
	bool ret = false;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		task_t* task = task_current_unsafe();
		uint16_t start = timer_get_current_unsafe();
		// If there's not enough data in the buffer and we're allowed to then we wait
		// for some more bytes to arrive
		while(fifo_size(task->recv_fifo) < count && !timer_has_elapsed_unsafe(start, wait_ms)) {
			// Set up task wait status
			task->status |= TASK_WAIT_RECV;
			if(wait_ms != TIMER_INFINITE) {
				// Set up a timeout as well
				task->status |= TASK_WAIT_TIMER;
				task->wait_until = start + wait_ms;
			}

			// Yield execution -> this will return only when either some more bytes
			// were received or the timeout was reached
			task_schedule_unsafe();
		}

		// If enough bytes are available, copy to output buffer
		ret = fifo_pop_bytes(task->recv_fifo, dest, count);
	}
	return ret;
}
#endif

#ifndef NO_USART_SEND
bool usart_send_bytes(const uint8_t* src, size_t count, uint16_t wait_ms) {
	bool ret = false;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		task_t* task = task_current_unsafe();
		uint16_t start = timer_get_current_unsafe();
		// If there's not enough free space in the buffer and we're allowed to then
		// we wait for some bytes to leave from the buffer
		while(fifo_available(task->send_fifo) < count && !timer_has_elapsed_unsafe(start, wait_ms)) {
			// Set up task wait status
			task->status |= TASK_WAIT_SEND;
			if(wait_ms != TIMER_INFINITE) {
				// Set up a timeout as well
				task->status |= TASK_WAIT_TIMER;
				task->wait_until = start + wait_ms;
			}

			// Yield execution -> this will return only when either some more bytes
			// were sent or the timeout was reached
			task_schedule_unsafe();
		}

		// If enough space is available, copy from input buffer
		ret = fifo_push_bytes(task->send_fifo, src, count);
		if(ret) {
			usart_send_on();
		}
	}
	return ret;
}
#endif

#endif // NO_USART
