/**
 * @file usart.h
 * Buffered USART communication library.
 * It supports simple HDLC-like framing with CRC-8 error detection, no-copy
 * operation for both input and output messages.
 *
 * @copyright (C) 2017 Peter Budai
 */

#ifndef _USART_H_
#define _USART_H_

#ifndef NO_USART

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * Initialize USART for transmit and receive.
 * Baud rate will be 38400 and frame format is 8N1.
 */
void usart_init(void);

/// Stops USART reception and transmission.
void usart_stop(void);

#ifndef NO_USART_RECV
/**
 * Returns or waits for the next count number of received bytes from the
 * input queue. If there is not enough received bytes available, it waits for
 * at most the given period of time for them to arrive.
 *
 * @param wait_ms Maximum number of milliseconds to wait for a message to arrive.
 *     Value of 0 will make this function non-blocking.
 *     Value of @ref TIMER_INFINITE will make the the function block indefinitely
 *     until the given number of bytes are available.
 * @return True if count bytes were successfully copied into buffer.
 *     False if less than count bytes were available until wait_ms elapsed.
 */
bool usart_receive_bytes(uint8_t* dest, size_t count, uint16_t wait_ms);
#endif

#ifndef NO_USART_SEND
/**
 * Sends or waits for the next count number of bytes to be placed into the
 * output queue. If the output queue is full, it waits for at most the given
 * period of time for enough space to become available.
 *
 * @param wait_ms Maximum number of milliseconds to wait for a message to be sent.
 *     Value of 0 will make this function non-blocking.
 *     Value of @ref TIMER_INFINITE will make the the function block indefinitely
 *     until the whole message fits into the output queue.
 * @return True if count bytes were successfully placed in the output queue.
 *     False if less than count bytes of space were available until wait_ms elapsed.
 */
bool usart_send_bytes(const uint8_t* src, size_t count, uint16_t wait_ms);
#endif

#endif // NO_USART

#endif // _USART_H_
