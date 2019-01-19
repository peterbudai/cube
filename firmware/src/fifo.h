/**
 * @file fifo.h
 * Simple byte queue for USART communication.
 *
 * @copyright (C) 2018 Peter Budai
 */

#ifndef _FIFO_H_
#define _FIFO_H_

#ifndef NO_USART

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/// Circular-buffer FIFO structure.
typedef struct fifo {
    /// Pointer to the whole buffer data area.
    uint8_t* buffer;
    /// Capacity of the whole buffer.
    size_t capacity;
    // Index where the first unread data byte starts.
    size_t start;
    /// Number of bytes currently in the buffer.
    size_t size;
    /// Index of the next transfer operation (pop or push).
    size_t current;
    /// Number of bytes affected by the current transfer operation (pop or push).
    size_t count;
} fifo_t;

#define fifo_size(fifo) ((fifo)->size)
#define fifo_available(fifo) ((fifo)->capacity - (fifo)->size)

/// Initialize an empty FIFO using the underlying buffer.
void fifo_init(fifo_t* fifo, uint8_t* buffer, size_t capacity);

/// Clears the FIFO.
void fifo_clear(fifo_t* fifo);

bool fifo_begin_push(fifo_t* fifo, size_t count);
void fifo_push(fifo_t* fifo, uint8_t data);
void fifo_commit_push(fifo_t* fifo);

bool fifo_push_bytes(fifo_t* fifo, const uint8_t* src, size_t count);

bool fifo_begin_pop(fifo_t* fifo, size_t count);
uint8_t fifo_peek(fifo_t* fifo);
uint8_t fifo_pop(fifo_t* fifo);
void fifo_commit_pop(fifo_t* fifo);

bool fifo_pop_bytes(fifo_t* fifo, uint8_t* dest, size_t count);

#endif // NO_USART

#endif // _FIFO_H_
