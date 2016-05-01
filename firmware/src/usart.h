#ifndef USART_H_
#define USART_H_

#include <stdbool.h>
#include <stdint.h>

#define USART_BUFFER_SIZE 192

/**
 * Initialize USART for transmit and receive.
 * Baud rate will be 38400 and frame format is 8N1.
 */
void usart_init();

void usart_stop();

bool usart_send_message_byte(uint8_t data);

bool usart_send_message_buf(uint8_t* buf, uint8_t length);

uint8_t usart_get_received_message_length();

uint8_t usart_get_received_message_byte(uint8_t index);

void usart_get_received_message_buf(uint8_t index, uint8_t* buf, uint8_t length);

void usart_drop_received_message();


#endif
