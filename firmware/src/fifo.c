#include "fifo.h"

void fifo_init(fifo_t* fifo, uint8_t* buffer, size_t capacity) {
    fifo->buffer = buffer;
    fifo->capacity = capacity;
    fifo_clear(fifo);
}

void fifo_clear(fifo_t* fifo) {
    fifo->start = 0;
    fifo->size = 0;
}
