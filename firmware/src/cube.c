#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/atomic.h>

#include "cube.h"
#include "draw.h"
#include "main.h"

// @name Port numbers and bits.
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

// @name Macros for dealing with output ports.
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

// Helper macros
#define frame_address(f) (frame_buffer + (f) * CUBE_FRAME_SIZE)
#define frame_next(f) ((f) >= CUBE_FRAME_BUFFER_COUNT - 1 ? 0 : (f) + 1)
#define frame_free() (edited_frame < current_frame ? current_frame - edited_frame : CUBE_FRAME_BUFFER_COUNT - (edited_frame - current_frame))

// Global variables.
uint8_t frame_buffer[CUBE_FRAME_SIZE * CUBE_FRAME_BUFFER_COUNT] __attribute__((section(".noinit")));
uint8_t current_layer __attribute__((section(".noinit")));
uint8_t current_repeat __attribute__((section(".noinit")));
uint8_t current_frame __attribute__((section(".noinit")));
uint8_t edited_frame __attribute__((section(".noinit")));

ISR(TIMER0_COMPA_vect) {
	// Display the current layer of the current frame
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

	// Advance to the next layer, iteration or frame
	current_layer++;
	if(current_layer >= 8) {
		current_layer = 0;
		current_repeat++;
		if(current_repeat >= CUBE_FRAME_REPEAT) {
			current_repeat = 0;
			// If there are no more frames to display, the last one will be freezed
			uint8_t next_frame = frame_next(current_frame);
			if(next_frame != edited_frame) {
				current_frame = next_frame;
			}
		}
	}
}

void cube_init(void)
{
	// Init I/O ports
	DDRB |= LAYER_MASK;
	DDRC |= (ROWL_MASK | SHIFT_BIT | STORE_BIT);
	DDRD |= (ROWH_MASK | ENABLE_BIT);

	// Set CTC mode
	TCCR0A = WGM01;
	// Set interval to approx 1000 Hz
	OCR0A = (F_CPU / 64 / (CUBE_FRAME_PER_SECOND * CUBE_FRAME_REPEAT * 8)) - 1;

	// Start with an empty frame
	current_frame = 0;
	edited_frame = 1;
	clear_frame(frame_address(current_frame));
}

void cube_enable(void)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		// Enable timer interrupt
		TIMSK0 = OCIE0A;
		// Set clock source to F_CPU/64
		TCCR0B = CS01 | CS00;
		// Reset timer
		TCNT0 = 0x00;
		// Start with a whole frame, cube will be enabled when the timer fires
		current_layer = 0;
		current_repeat = 0;
	}
}

void cube_disable(void)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		// Set clock source to OFF
		TCCR0B = 0x00;
		// Disable timer interrupt
		TIMSK0 = 0x00;
		// Clear interrupt flag if any
		TIFR0 = OCF0A;
		// Turn cube off
		enable_off();
	}
}


uint8_t cube_get_free_frames(void) {
	uint8_t free_count;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		free_count = frame_free() - 1;
	}
	return free_count;
}

uint8_t* cube_advance_frame(void) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		uint8_t next_frame = frame_next(edited_frame);
		while(next_frame == current_frame) {
			sleep_enable();
			sei();
			sleep_cpu();

			next_frame = frame_next(edited_frame);
		}
		edited_frame = next_frame;
	}
	return frame_address(edited_frame);
}

