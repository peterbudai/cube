#ifndef IO_H
#define IO_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

extern volatile uint64_t mcu_ticks;

#define LED_COUNT 8

void leds_copy_state(uint8_t state[][LED_COUNT], bool* enabled);
void leds_set_layer(uint8_t layer, uint8_t value[]);
void leds_dim_up(void);
void leds_dim_down(void);

#define UART_BUFFER_SIZE 256

typedef enum {
	UART_INPUT = 0,
	UART_OUTPUT = 1
} uart_dir_t;

void uart_get_counts(uint64_t* counts, uint64_t* drops);
bool uart_empty(uart_dir_t dir);
void uart_push_back(uart_dir_t dir, uint8_t* data, size_t count);
size_t uart_peek_front(uart_dir_t dir, uint8_t* data, size_t size);
void uart_pop_front(uart_dir_t dir, size_t count);
bool uart_get_front(uart_dir_t dir, uint8_t* data);

void io_init(void);

#endif

