#ifndef USART_H
#define USART_H

#include <stdbool.h>
#include <stdint.h>

#define USART_INPUT_BUFFER_COUNT 24
#define USART_OUTPUT_BUFFER_COUNT 8
#define USART_MESSAGE_SIZE 15

typedef struct {
	uint8_t header;
	uint8_t body[USART_MESSAGE_SIZE];
} usart_message_t;

#define usart_get_message_type(message) ((message).header >> 4)
#define usart_get_message_length(message) ((message).header & USART_MESSAGE_SIZE)
#define usart_set_message_header(message, type, length) ((message).header = ((type) << 4) | ((length) & USART_MESSAGE_SIZE))

/**
 * Initialize USART for transmit and receive.
 * Baud rate will be 38400 and frame format is 8N1.
 */
void usart_init(void);
void usart_stop(void);

usart_message_t* usart_receive_input_message(uint16_t wait_ms);
bool usart_drop_input_message(void);
usart_message_t* usart_prepare_output_message(uint16_t wait_ms);
bool usart_send_output_message(void);

#endif
