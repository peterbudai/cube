#include "cube.h"

#ifndef NO_CUBE

#include <stdlib.h>
#include <avr/io.h>
#include <util/atomic.h>

#include "cpu.h"
#include "draw.h"
#include "task.h"
#include "timer.h"

// Port numbers and bits.
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

// Macros for dealing with output ports.
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

// Global variables
uint8_t frame_buffer[CUBE_FRAME_SIZE * CUBE_FRAME_BUFFER_COUNT];
uint8_t current_layer;
uint8_t current_repeat;
uint8_t current_frame;
uint8_t edited_frame;
bool enabled;

bool cube_refresh(void) {
	// When cube is turned off, do not consume resources
	if(!enabled) {
		return false;
	}

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
	// Assuming that this function is called once per milliseconds, a full frame
	// requires 8 ms to display once, but will be repeteated before the next
	// frame comes in every 40 ms, thus we get a nice 25 Hz frame rate which
	// suits well for displaying fluid animations.

	// Going through all layers
	current_layer++;
	if(current_layer < 8) {
		return false;
	}
	current_layer = 0;

	// Each full frame is repeated 5 times to help the image stabilize visually
	current_repeat++;
	if(current_repeat < 5) {
		return false;
	}
	current_repeat = 0;

	// If there are no more frames to display, the last one will be freezed
	uint8_t next_frame = frame_next(current_frame);
	if(next_frame == edited_frame) {
		return false;
	}
	current_frame = next_frame;

	// Successful frame switch: wake up tasks waiting for cube
	bool wake = false;
	for(uint8_t i = 0; i < TASK_COUNT; ++i) {
		if(tasks[i].status & TASK_WAIT_CUBE) {
			tasks[i].status &= ~TASK_WAITING;
			wake = true;
		}
	}
	return wake;
}

void cube_init(void)
{
	// Init I/O ports
	DDRB |= LAYER_MASK;
	DDRC |= (ROWL_MASK | SHIFT_BIT | STORE_BIT);
	DDRD |= (ROWH_MASK | ENABLE_BIT);

	// Start with an empty frame
	enabled = false;
	current_frame = 0;
	edited_frame = 1;
	clear_frame(frame_address(current_frame));
}

void cube_enable(void)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		// Start with a whole frame, cube will be enabled when the timer fires
		current_layer = 0;
		current_repeat = 0;
		// Turn on timer event processing
		enabled = true;
	}
}

void cube_disable(void)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		// Turn off timer event processing
		enabled = false;
		// Turn off cube outputs
		enable_off();
	}
}

uint8_t cube_get_free_frames(void) {
	uint8_t free_count;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		if(edited_frame < current_frame) {
			free_count = current_frame - edited_frame;
		} else {
			free_count = CUBE_FRAME_BUFFER_COUNT - (edited_frame - current_frame);
		}
	}
	return free_count - 1;
}

uint8_t* cube_advance_frame(uint16_t wait_ms) {
	uint8_t* ret = NULL;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		uint8_t next_frame = frame_next(edited_frame);

		// If there's no free frame and we're allowed to then we wait for some
		// frames to be displayed and become free for editing
		if(next_frame == current_frame && wait_ms > 0) {
			// Set up task wait status
			task_t* task = task_current_unsafe();
			task->status |= TASK_WAIT_CUBE;
			if(wait_ms != TIMER_INFINITE) {
				// Set up a timeout as well
				task->status |= TASK_WAIT_TIMER;
				task->wait_until = timer_get_current_unsafe() + wait_ms;
			}

			// Yield execution -> this will return only when either a free frame is
			// available or the timeout was reached
			task_schedule_unsafe();
		}

		// If there is an available frame, return its address
		if(next_frame != current_frame) {
			edited_frame = next_frame;
			ret = frame_address(edited_frame);
		}
	}
	return ret;
}

#endif // NO_CUBE
