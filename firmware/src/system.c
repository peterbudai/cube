#include "system.h"

#include <setjmp.h>
#include <util/atomic.h>

#include "app.h"
#include "cpu.h"
#include "timer.h"

#define SYSTEM_MESSAGE_TYPE 0x0F

jmp_buf current_context __attribute__((section(".noinit")));
uint8_t current_app __attribute__((section(".noinit")));
uint8_t current_command __attribute__((section(".noinit")));

void system_handle_timer(void) {
	cpu_check_stack();
}

bool system_handle_usart_input(usart_message_t* message) {
	// Non-system messages should be processed as usual
	if(usart_get_message_type(*message) != SYSTEM_MESSAGE_TYPE) {
		return true;
	}

	if(usart_get_message_length(*message) == 1) {
		// Save command for later processing
		current_command = message->body[0];
	}

	// System messages are dropped immediately
	return false;
}

bool system_handle_usart_output(usart_message_t* message) {
	return false;
}

void system_handle_events(void) {
	cpu_check_stack();

	uint8_t command = 0;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		command = current_command;
		current_command = 0;

		if((command & 0x80) != 0) {
			current_app = command & 0x7F;
		}
	}

	if((command & 0x80) != 0) {
		longjmp(current_context, 255);
	}

	if(command != 0) {
		longjmp(current_context, command);
	}
}

bool system_run(void) {
	current_app = 0;
	current_command = 0;

	while(true) {
		switch(setjmp(current_context)) {
			case 0:
				// Start the selected app
				apps[current_app]();
				break;
			case 1:
				// Asked to shut down
				return false;
			case 2:
				// Asked to reset
				return true;
			case 255:
				// The app has been terminated, reset subsystems before running the next one
				break;
		}
	}
}
