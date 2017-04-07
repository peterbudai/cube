#include "io.h"

#include <stdlib.h>
#include <string.h>

#include <pthread.h>

// MCU state
volatile uint64_t mcu_ticks = 0;

// Leds
int leds_enabled = 0;
uint8_t leds_state[LED_COUNT][LED_COUNT];
pthread_mutex_t leds_mutex;

void leds_copy_state(uint8_t state[][LED_COUNT], bool* enabled) {
	pthread_mutex_lock(&leds_mutex);
	memcpy(state, leds_state, sizeof(leds_state));
	*enabled = leds_enabled > 0;
	pthread_mutex_unlock(&leds_mutex);
}

void leds_set_layer(uint8_t layer, uint8_t value[]) {
	pthread_mutex_lock(&leds_mutex);
	for(size_t i = 0; i < LED_COUNT; ++i) {
		leds_state[layer][i] = value[i];
	}
	pthread_mutex_unlock(&leds_mutex);
}

void leds_dim_up(void) {
	pthread_mutex_lock(&leds_mutex);
	leds_enabled = 8;
	pthread_mutex_unlock(&leds_mutex);
}

void leds_dim_down(void) {
	pthread_mutex_lock(&leds_mutex);
	if(leds_enabled > 0) {
		leds_enabled--;
	}
	pthread_mutex_unlock(&leds_mutex);
}

// UART

typedef struct {
	uint8_t data[UART_BUFFER_SIZE];
	size_t size;
	size_t start;
	uint64_t count;
	uint64_t dropped;
} uart_buffer_t;

uart_buffer_t uart_buffers[2];
pthread_mutex_t uart_mutex;

void uart_get_counts(uint64_t* counts, uint64_t* drops) {
	pthread_mutex_lock(&uart_mutex);
	for(size_t i = UART_INPUT; i <= UART_OUTPUT; ++i) {
		counts[i] = uart_buffers[i].count;
		drops[i] = uart_buffers[i].dropped;
	}
	pthread_mutex_unlock(&uart_mutex);
}

bool uart_empty(uart_dir_t dir) {
	pthread_mutex_lock(&uart_mutex);
	bool empty = (uart_buffers[dir].size == 0);
	pthread_mutex_unlock(&uart_mutex);
	return empty;
}

void uart_push_back(uart_dir_t dir, uint8_t* data, size_t count) {
	pthread_mutex_lock(&uart_mutex);
	uart_buffer_t* buf = &(uart_buffers[dir]);
	size_t end = buf->start + buf->size % UART_BUFFER_SIZE;
	for(size_t i = 0; i < count; ++i) {
		buf->data[end++] = data[i];
		if(end == UART_BUFFER_SIZE) {
			end = 0;
		}
		if(buf->size < UART_BUFFER_SIZE) {
			buf->size++;
		} else {
			buf->dropped++;
			if(buf->start++ == UART_BUFFER_SIZE) {
				buf->start = 0;
			}
		}
		buf->count++;
	}
	pthread_mutex_unlock(&uart_mutex);
}

size_t uart_peek_front(uart_dir_t dir, uint8_t* data, size_t size) {
	pthread_mutex_lock(&uart_mutex);
	uart_buffer_t* buf = &(uart_buffers[dir]);
	if(size > buf->size) {
		size = buf->size;
	}
	size_t cur = buf->start;
	for(size_t i = 0; i < size; ++i) {
		data[i] = buf->data[cur++];
		if(cur >= UART_BUFFER_SIZE) {
			cur = 0;
		}
	}
	pthread_mutex_unlock(&uart_mutex);
	return size;
}

void uart_pop_front(uart_dir_t dir, size_t count) {
	pthread_mutex_lock(&uart_mutex);
	uart_buffer_t* buf = &(uart_buffers[dir]);
	if(count > buf->size) {
		count = buf->size;
	}
	buf->size -= count;
	buf->start = (buf->start + count) % UART_BUFFER_SIZE;
	pthread_mutex_unlock(&uart_mutex);
}

void io_init(void) {
	memset(leds_state, 0, sizeof(leds_state));
	pthread_mutex_init(&leds_mutex, NULL);

	memset(uart_buffers, 0, sizeof(uart_buffers));
	pthread_mutex_init(&uart_mutex, NULL);
}

