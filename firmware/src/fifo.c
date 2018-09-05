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

static size_t fifo_normalize(fifo_t* fifo, size_t index) {
    return (index < fifo->capacity) ? index : (index - fifo->capacity);
}

bool fifo_begin_push(fifo_t* fifo, size_t count) {
    if(fifo_available(fifo) < count) {
        return false;
    }
    fifo->count = 0;
    fifo->current = fifo_normalize(fifo, fifo->start + fifo->size);
    return true;
}

void fifo_push(fifo_t* fifo, uint8_t data) {
    fifo->buffer[fifo->current] = data;
    fifo->current = fifo_normalize(fifo, fifo->current + 1);
    fifo->count++;
}

void fifo_commit_push(fifo_t* fifo) {
    fifo->size += fifo->count;
    fifo->count = 0;
}

bool fifo_push_bytes(fifo_t* fifo, const uint8_t* src, size_t count) {
    if(fifo_available(fifo) < count) {
        return false;
    }
    size_t dest = fifo_normalize(fifo, fifo->start + fifo->size);
    for(size_t i = 0; i < count; ++i) {
        fifo->buffer[fifo_normalize(fifo, dest + i)] = src[i];
    }
    fifo->size += count;
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
    uint8_t data = fifo->buffer[fifo->current];
    fifo->current = fifo_normalize(fifo, fifo->current + 1);
    fifo->count++;
    return data;
}

void fifo_commit_pop(fifo_t* fifo) {
    fifo->start = fifo_normalize(fifo, fifo->start + fifo->count);
    fifo->size -= fifo->count;
    fifo->count = 0;
}

bool fifo_pop_bytes(fifo_t* fifo, uint8_t* dest, size_t count) {
    if(fifo_size(fifo) < count) {
        return false;
    }
    size_t src = fifo->start;
    for(size_t i = 0; i < count; ++i) {
        dest[i] = fifo->buffer[fifo_normalize(fifo, src + i)];
    }
    fifo->start = fifo_normalize(fifo, fifo->start + count);
    fifo->size -= count;
    return true;
}

