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

#include <stdbool.h>
#include <stdint.h>

/// Number of input messages that can be queued without dropping a frame.
#define USART_INPUT_BUFFER_COUNT 8
/// Number of output messages that can be queued without dropping a frame.
#define USART_OUTPUT_BUFFER_COUNT 8
/// Number of body bytes that can maximally fit in a message.
#define USART_MESSAGE_SIZE 15

/// Generic message data structure for input and output.
typedef struct {
	/**
	 * Message header. This part is always present on the wire.
	 * The upper 4 bits is the message type and the lower 4 bits is the
	 * body length. Use usart_get_message_type(), usart_get_message_length(),
	 * and usart_set_message_header() macros to manipulate it.
	 */
	uint8_t header;
	/**
	 * Message body. It is optional, its length can be between 0 and
	 * @ref USART_MESSAGE_SIZE.
	 */
	uint8_t body[USART_MESSAGE_SIZE];
} usart_message_t;

/// Extract the type bits from the message header.
#define usart_get_message_type(message) ((message).header >> 4)
/// Extract the body length from the message header.
#define usart_get_message_length(message) ((message).header & USART_MESSAGE_SIZE)
/// Set type and body length in the message header.
#define usart_set_message_header(message, type, length) ((message).header = ((type) << 4) | ((length) & USART_MESSAGE_SIZE))

/**
 * Initialize USART for transmit and receive.
 * Baud rate will be 38400 and frame format is 8N1.
 */
void usart_init(void);

/// Stops USART reception and transmission.
void usart_stop(void);

/**
 * Returns or waits for the next received message from input queue.
 * If there is no received message, it waits for at most the given period
 * of time for one to arrive. However, system events are still handled
 * in the meanwhile.
 *
 * @param wait_ms Maximum number of milliseconds to wait for a message to arrive.
 *     Value of 0 will make this function non-blocking.
 *     Value of @ref TIMER_INFINITE will make the the function block indefinitely
 *     until a message is available.
 * @return Address of the received message. NULL if no message arrived during
 *     the wait period. Call usart_drop_input_message() after the message has
 *     been processed to make its buffer space available for receiving again.
 *     Otherwise calling this function again will return the same message.
 */
usart_message_t* usart_receive_input_message(uint16_t wait_ms);

/**
 * Discards the input message at the beginning of the queue.
 * Call this function after you finished processing the message obtained from
 * usart_receive_input_message().
 * However, this function allows to discard messages without processing it
 * first, so take extra caution not to discard important messages accidentally.
 *
 * @return True if a message was discarded successfully, false if the input
 *     queue is empty.
 */
bool usart_drop_input_message(void);

/**
 * Obtains or waits for and empty message for editing from output queue.
 * If the output queue is full, it waits for at most the given period
 * of time for a message to become available. However, system events are still
 * handled in the meanwhile.
 *
 * @param wait_ms Maximum number of milliseconds to wait for a message to edit.
 *     Value of 0 will make this function non-blocking.
 *     Value of @ref TIMER_INFINITE will make the the function block indefinitely
 *     until a message is available.
 * @return Address of the message to edit. NULL if the output queue is still
 *     full after the wait period. Call usart_send_output_message() after the
 *     message has been fully assembled to queue it for sending. Otherwise
 *     calling this function again will return the same message.
 */
usart_message_t* usart_prepare_output_message(uint16_t wait_ms);

/**
 * Queues the currently edited output message for sending.
 * Call this function after you finished assembling the message obtained from
 * usart_prepare_output_message(). This will promote a new message for editing.
 * However, this function allows to send messages without properly assembling
 * it first, so take extra caution not to send unfinished messages accidentally
 * that may contain memory garbage, or previously sent messages.
 *
 * @return True if a message was queued successfully, false if the output
 *     queue is full. In this case, calling this function again will try to
 *     send the same message again.
 */
bool usart_send_output_message(void);

#endif
