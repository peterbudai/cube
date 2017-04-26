#ifndef USART_H
#define USART_H

#include <stdbool.h>
#include <stdint.h>

#define USART_INPUT_BUFFER_COUNT 24
#define USART_OUTPUT_BUFFER_COUNT 8
#define USART_MESSAGE_SIZE 16

typedef struct {
	uint8_t type : 4;
	uint8_t length : 4;
	uint8_t body[USART_MESSAGE_SIZE];
} usart_message_t;

/**
 * Initialize USART for transmit and receive.
 * Baud rate will be 38400 and frame format is 8N1.
 */
void usart_init(void);
void usart_stop(void);

usart_message_t* usart_receive_input_message(uint16_t wait_ms);
usart_message_t* usart_get_output_message(void);
usart_message_t* usart_send_output_message(uint16_t wait_ms);

#endif /* USART_H */
