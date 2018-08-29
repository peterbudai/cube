/**
 * @file fifo.h
 * Simple byte queue for USART communication.
 *
 * @copyright (C) 2018 Peter Budai
 */

#ifndef FIFO_H
#define FIFO_H

#include <stddef.h>
#include <stdint.h>

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

void fifo_init(fifo_t* fifo, uint8_t* buffer, size_t capacity);
void fifo_clear(fifo_t* fifo);

#endif