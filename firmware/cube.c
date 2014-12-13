#include "cube.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

// Port numbers and bits
#define ROWH_PORT PORTD
#define ROWH_MASK ((1 << PORTD7) | (1 << PORTD6) | (1 << PORTD5) | (1 << PORTD4))
#define ROWL_PORT PORTC
#define ROWL_MASK ((1 << PORTC3) | (1 << PORTC2) | (1 << PORTC1) | (1 << PORTC0))

#define ENABLE_PORT PORTD
#define ENABLE_BIT (1 << PORTD3)

#define SHIFT_PORT PORTC
#define SHIFT_BIT (1 << PORTC4)
#define STORE_PORT PORTC
#define STORE_BIT (1 << PORTC5)

#define LAYER_PORT PORTB
#define LAYER_MASK ((1 << PORTB2) | (1 << PORTB1) | (1 << PORTB0))

#define FRAME_REPEAT 5
#define FRAME_PER_SECOND 25

uint8_t frame_buffer[CUBE_FRAME_SIZE * CUBE_FRAME_BUFFER_COUNT] __attribute__ ((section (".noinit")));

// Macros for dealing with output ports
#define enable_off() ENABLE_PORT |= ENABLE_BIT
#define enable_on() ENABLE_PORT &= ~ENABLE_BIT

#define shift() \
	SHIFT_PORT |= SHIFT_BIT; \
	SHIFT_PORT &= ~(SHIFT_BIT)
#define store() \
	STORE_PORT |= STORE_BIT; \
	STORE_PORT &= ~(STORE_BIT)

#define layer_select(l) LAYER_PORT = (LAYER_PORT & ~(LAYER_MASK)) | ((l) & LAYER_MASK)
#define layer_address(l) ((l) * 8)

#define frame_address(f) (frame_buffer + (f) * CUBE_FRAME_SIZE)
#define frame_next(f) ((f) >= CUBE_FRAME_BUFFER_COUNT - 1 ? 0 : (f) + 1)
	
uint8_t current_layer __attribute__ ((section (".noinit")));		// 0..7
uint8_t current_repeat __attribute__ ((section (".noinit")));		// 0..FRAME_REPEAT
uint8_t current_frame __attribute__ ((section (".noinit")));		// 0..CUBE_FRAME_BUFFER_COUNT
uint8_t edited_frame __attribute__ ((section (".noinit")));			// 0..CUBE_FRAME_BUFFER_COUNT

ISR(TIMER0_COMPA_vect) {
	uint8_t* layer = frame_address(current_frame) + layer_address(current_layer);
	
	enable_off();
	layer_select(current_layer);
	for(uint8_t row = 0; row < 8; row++) {
		uint8_t columns = layer[row];
		ROWH_PORT = (ROWH_PORT & ~(ROWH_MASK)) | (columns & ROWH_MASK);
		ROWL_PORT = (ROWL_PORT & ~(ROWL_MASK)) | (columns & ROWL_MASK);
		shift();
	}
	store();
	enable_on();
	
	current_layer++;
	if(current_layer >= 8) {
		current_layer = 0;
		current_repeat++;
		if(current_repeat >= FRAME_REPEAT) {
			current_repeat = 0;
			uint8_t next_frame = frame_next(current_frame);
			if(next_frame != edited_frame) {
				current_frame = next_frame;
			}
		}
	}
}

static void cube_clear_edited_frame() {
	uint8_t* frame = frame_address(edited_frame);
	for(uint8_t i = 0; i < CUBE_FRAME_SIZE; i++) {
		frame[i] = 0;
	}
}

static void cube_copy_edited_frame(uint8_t next) {
	uint8_t* prev_frame = frame_address(edited_frame);
	uint8_t* next_frame = frame_address(next);
	for(uint8_t i = 0; i < CUBE_FRAME_SIZE; i++) {
		next_frame[i] = prev_frame[i];
	}
}

void cube_init()
{
	DDRB |= LAYER_MASK;
	DDRC |= (ROWL_MASK | SHIFT_BIT | STORE_BIT);
	DDRD |= (ROWH_MASK | ENABLE_BIT);

	current_frame = 0;
	edited_frame = 0;
	
	cube_clear_edited_frame();
	cube_advance_frame(CUBE_FRAME_CLEAR);

	TCCR0A = 2;		// Set CTC mode
	OCR0A = 0x7C;	// Set interval to 1000 Hz
}

bool cube_advance_frame(uint8_t method) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		uint8_t next_frame = frame_next(edited_frame);
		if(next_frame == current_frame) {
			return false;
		}
		if(method == CUBE_FRAME_COPY) {
			cube_copy_edited_frame(next_frame);
		}
		edited_frame = next_frame;
		if(method == CUBE_FRAME_CLEAR) {
			cube_clear_edited_frame();
		}
	}
	return true;
}

uint8_t* cube_get_frame() {
	return frame_address(edited_frame);
}

void cube_disable()
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		TCCR0B = 0;		// Set clock source to OFF
		TIMSK0 = 0;		// Disable timer interrupt
		TIFR0 = 2;		// Clear interrupt flag if any
		enable_off();
	}
}

void cube_enable()
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		TIMSK0 = 2;		// Enable timer interrupt
		TCCR0B = 3;		// Set clock source to F_CPU/64
		TCNT0 = 0;		// Reset timer

		current_layer = 0;
		current_repeat = 0;
	}
}
