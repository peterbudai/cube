#ifndef IO_H
#define IO_H

#include <stdbool.h>
#include <stdint.h>

#include <pthread.h>

extern volatile uint64_t mcu_ticks;

#define LED_COUNT 8

void leds_init(void);
void leds_copy_state(uint8_t state[][LED_COUNT], bool* enabled);
void leds_set_layer(uint8_t layer, uint8_t value[]);
void leds_dim_up(void);
void leds_dim_down(void);

#define UART_BUFFER_SIZE 64

typedef struct {
	uint8_t data[UART_BUFFER_SIZE];
	size_t size;
	size_t start;
	pthread_mutex_t mutex;
} uart_buffer_t;

#endif

