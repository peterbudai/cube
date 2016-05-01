/// @file cube.c
/// @copyright (C) 2016 Peter Budai
/// LED cube peripherial control code.

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#include "cube.h"

/// @name Port numbers and bits.
/// @{
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
/// @}

/// @name Macros for dealing with output ports.
/// @{
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
/// @}

/// How many times a full frame is displayed before advancing to the next one.
#define FRAME_REPEAT 5
/// How many frames will be displayed in a second.
#define FRAME_PER_SECOND 25

/// Calculates the address of current frame.
/// @param f The current frame index.
#define frame_address(f) (frame_buffer + (f) * CUBE_FRAME_SIZE)
/// Calculates the address of the next frame.
/// @param f The current frame index.
#define frame_next(f) ((f) >= CUBE_FRAME_BUFFER_COUNT - 1 ? 0 : (f) + 1)

/// @name Global variables.
/// @{

/// Frame buffer.
/// This is a circular buffer for CUBE_FRAME_BUFFER_COUNT number of full 3D frames.
uint8_t frame_buffer[CUBE_FRAME_SIZE * CUBE_FRAME_BUFFER_COUNT] __attribute__((section(".noinit")));
/// Current layer being multiplexed.
/// Possible values: 0..7
uint8_t current_layer __attribute__((section(".noinit")));
/// Current iteration number of the currently displayed 3D frame.
/// Possible values: 0..FRAME_REPEAT
uint8_t current_repeat __attribute__((section(".noinit")));
/// The index of the currently displayed 3D frame in the frame buffer.
/// Possible values: 0..CUBE_FRAME_BUFFER_COUNT
uint8_t current_frame __attribute__((section(".noinit")));
/// The index of the currently edited 3D frame in the frame buffer.
/// Possible values: 0..CUBE_FRAME_BUFFER_COUNT
uint8_t edited_frame __attribute__((section(".noinit")));
/// How many frames are filled in the buffer (including the displayed one, but not the edited one).
/// Possible values: 0..CUBE_FRAME_BUFFER_COUNT
uint8_t frame_count __attribute__((section(".noinit")));

/// @}

/// Timer interrupt handler.
/// This will fire once the next layer has to be displayed.
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
		if(current_repeat >= FRAME_REPEAT) {
			current_repeat = 0;
			// If there are no more frames to display, the last one will be freezed
			if(frame_count > 1) {
				current_frame = frame_next(current_frame);
				frame_count--;
			}
		}
	}
}

/// Clears the currently edited frame
static void cube_clear_edited_frame(void) {
	uint8_t* frame = frame_address(edited_frame);
	for(uint8_t i = 0; i < CUBE_FRAME_SIZE; i++) {
		frame[i] = 0;
	}
}

/// Copies the contents of the currently edited frame to the next one.
/// @param next Index of the next frame.
static void cube_copy_edited_frame(uint8_t next) {
	uint8_t* prev_frame = frame_address(edited_frame);
	uint8_t* next_frame = frame_address(next);
	for(uint8_t i = 0; i < CUBE_FRAME_SIZE; i++) {
		next_frame[i] = prev_frame[i];
	}
}

void cube_init(void)
{
	// Init I/O ports
	DDRB |= LAYER_MASK;
	DDRC |= (ROWL_MASK | SHIFT_BIT | STORE_BIT);
	DDRD |= (ROWH_MASK | ENABLE_BIT);

	// Start with an empty frame
	current_frame = 0;
	edited_frame = 0;
	cube_clear_edited_frame();
	cube_advance_frame(CUBE_FRAME_CLEAR);

	// Set CTC mode
	TCCR0A = WGM01;
	// Set interval to approx 1000 Hz
	OCR0A = (F_CPU / 64 / (FRAME_PER_SECOND * FRAME_REPEAT * 8)) - 1;
}

uint8_t cube_advance_frame(uint8_t method) {
	uint8_t free_count;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		free_count = CUBE_FRAME_BUFFER_COUNT - frame_count;
		// If there are no more undisplayed frames, the current one remains being edited
		if(free_count > 0) {
			uint8_t next_frame = frame_next(edited_frame);
			if(method == CUBE_FRAME_COPY) {
				cube_copy_edited_frame(next_frame);
			}
			edited_frame = next_frame;
			frame_count++;
			if(method == CUBE_FRAME_CLEAR) {
				cube_clear_edited_frame();
			}
		}
	}
	return free_count;
}

uint8_t* cube_get_frame(void) {
	return frame_address(edited_frame);
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

