#ifndef USART_H_
#define USART_H_

#ifndef BAUD
#define BAUD 38400
#endif

/**
 * Initialize USART for transmit and receive.
 * Baud rate will be BAUD and frame format is 8N1.
 */
void usart_init();

#endif
