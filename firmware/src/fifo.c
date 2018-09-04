#include "fifo.h"

void fifo_init(fifo_t* fifo, uint8_t* buffer, size_t capacity) {
    fifo->buffer = buffer;
    fifo->capacity = capacity;
    fifo_clear(fifo);
}

void fifo_clear(fifo_t* fifo) {
    fifo->start = 0;
    fifo->size = 0;
    fifo->current = 0;
    fifo->count = 0;
}

#define fifo_normalize(fifo, index) if((fifo)->index >= (fifo)->capacity) { (fifo)->index -= (fifo)->capacity; }

bool fifo_begin_push(fifo_t* fifo, size_t count) {
    if(fifo_available(fifo) < count) {
        return false;
    }
    fifo->count = 0;
    fifo->current = fifo->start + fifo->size;
    fifo_normalize(fifo, current);
    return true;
}

void fifo_push(fifo_t* fifo, uint8_t data) {
    fifo->buffer[fifo->current++] = data;
    fifo_normalize(fifo, current);
    fifo->count++;
}

void fifo_commit_push(fifo_t* fifo) {
    fifo->size += fifo->count;
    fifo->count = 0;
}

bool fifo_push_bytes(fifo_t* fifo, const uint8_t* src, size_t count) {
    if(!fifo_begin_push(fifo, count)) {
        return false;
    }
    while(count > 0) {
        fifo_push(fifo, *(src++));
        count--;
    }
    fifo_commit_push(fifo);
    return true;
}

bool fifo_begin_pop(fifo_t* fifo, size_t count) {
    if(fifo_size(fifo) < count) {
        return false;
    }
    fifo->count = 0;
    fifo->current = fifo->start;
    return true;
}

uint8_t fifo_peek(fifo_t* fifo) {
    return fifo->buffer[fifo->current];
}

uint8_t fifo_pop(fifo_t* fifo) {
    uint8_t data = fifo->buffer[fifo->current++];
    fifo_normalize(fifo, current);
    fifo->count++;
    return data;
}

void fifo_commit_pop(fifo_t* fifo) {
    fifo->start += fifo->count;
    fifo_normalize(fifo, start);
    fifo->size -= fifo->count;
    fifo->count = 0;
}

bool fifo_pop_bytes(fifo_t* fifo, uint8_t* dest, size_t count) {
    if(!fifo_begin_pop(fifo, count)) {
        return false;
    }
    while(count > 0) {
        *(dest++) = fifo_pop(fifo);
        count--;
    }
    fifo_commit_pop(fifo);
    return true;
}

