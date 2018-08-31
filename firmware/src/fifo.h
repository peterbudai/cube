/**
 * @file fifo.h
 * Simple byte queue for USART communication.
 *
 * @copyright (C) 2018 Peter Budai
 */

#ifndef _FIFO_H_
#define _FIFO_H_

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
} fifo_t;

/// Initialize an empty FIFO using the underlying buffer.
void fifo_init(fifo_t* fifo, uint8_t* buffer, size_t capacity);

/// Clears the FIFO.
void fifo_clear(fifo_t* fifo);

#endif