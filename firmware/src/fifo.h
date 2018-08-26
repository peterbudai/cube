/**
 * @file fifo.h
 * Simple byte queue for USART communication.
 *
 * @copyright (C) 2018 Peter Budai
 */

#ifndef FIFO_H
#define FIFO_H

#include <stdint.h>

typedef struct {
    /// Pointer to the whole FIFO buffer
    uint8_t* buffer;
    /// Size of the whole buffer
    uint16_t capacity;
    // Index where the first complete message starts
    uint16_t start;
    /// Number of bytes currently in completed messages
    uint16_t size;
} fifo_t;

#endif