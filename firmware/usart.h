#ifndef USART_H_
#define USART_H_

#include <stdbool.h>
#include <stdint.h>

#define USART_BUFFER_SIZE 128

/**
 * Initialize USART for transmit and receive.
 * Baud rate will be 38400 and frame format is 8N1.
 */
void usart_init();

bool usart_send_byte(uint8_t data);

bool usart_send_buf(uint8_t* buf, uint8_t length);


#endif
