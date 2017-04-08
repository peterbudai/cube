#include "mcu.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>

#include <simavr/avr_ioport.h>
#include <simavr/avr_uart.h>
#include <simavr/sim_avr.h>
#include <simavr/sim_elf.h>
#include <simavr/sim_gdb.h>

#include "io.h"

// Constants
#define PORTB 0x25
#define PORTC 0x28
#define PORTD 0x2B

// Global variables
avr_t* mcu = NULL;
pthread_t mcu_thread;

avr_irq_t* input_irq = NULL;
bool input_on = false;
int input_overflow_prob = 0;
uint8_t shift_reg[LED_COUNT];

static void* mcu_run(void* args __attribute__((unused))) {
	int next_overflow_rand = rand();
	while(true) {
		++mcu_ticks;
		avr_run(mcu);

		if(input_on || next_overflow_rand < input_overflow_prob) {
			uint8_t data;
			if(!uart_empty(UART_INPUT) && uart_get_front(UART_INPUT, &data)) {
				avr_raise_irq(input_irq, data);
			}
			next_overflow_rand = rand();
		}
	}
	return NULL;
}

static void handle_leds_enable(struct avr_irq_t* irq __attribute__((unused)), uint32_t value, void* param __attribute__((unused))) {
	if(value == 0) {
		leds_dim_up();
	}
}

static void handle_leds_shift(struct avr_irq_t* irq __attribute__((unused)), uint32_t value, void* param __attribute__((unused))) {
	if(value != 0) {
		uint8_t row_data = (mcu->data[PORTD] & 0xF0) | (mcu->data[PORTC] & 0x0F);
		for(int i = 0; i < 8; ++i) {
			shift_reg[i] = (shift_reg[i] << 1) | ((row_data >> i) & 0x01);
		}
	}
}

static void handle_leds_store(struct avr_irq_t* irq __attribute__((unused)), uint32_t value, void* param __attribute__((unused))) {
	if(value != 0) {
		uint8_t layer = mcu->data[PORTB] & 0x07;
		leds_set_layer(layer, shift_reg);
	}
}

static void handle_uart_output(struct avr_irq_t* irq __attribute__((unused)), uint32_t value, void* param __attribute__((unused))) {
	uint8_t data = value;
	uart_push_back(UART_OUTPUT, &data, sizeof(data));
}

static void handle_uart_xon(struct avr_irq_t* irq __attribute__((unused)), uint32_t value, void* param __attribute__((unused))) {
	if(value == 1) {
		input_on = true;
	}
}

static void handle_uart_xoff(struct avr_irq_t* irq __attribute__((unused)), uint32_t value, void* param __attribute__((unused))) {
	if(value == 1) {
		input_on = false;
	}
}

void mcu_init(int argc, char** argv) {
	int gdb_port = 0;
	bool gdb_stopped = false;
	int drop_rate = 0;
	for(int i = 1; i < argc; ++i) {
		if(strcmp(argv[i], "-g") == 0) {
			if(i + 1 >= argc || (gdb_port = atoi(argv[++i])) <= 0) {
				printf("Missing or invalid GDB port number\n");
				exit(1);
			}
			continue;
		}
		if(strcmp(argv[i], "-p") == 0) {
			gdb_stopped = true;
			continue;
		}
		if(strcmp(argv[i], "-d") == 0) {
			if(i + 1 >= argc || (drop_rate = atoi(argv[++i])) <= 0 || drop_rate > 100) {
				printf("Missing or invalid drop rate\n");
				exit(1);
			}
			continue;
		}
	}

	memset(shift_reg, 0, sizeof(shift_reg));
	elf_firmware_t fw;
	elf_read_firmware("../firmware/out/firmware.elf", &fw);

	mcu = avr_make_mcu_by_name(MCU_NAME);
	avr_init(mcu);
	avr_load_firmware(mcu, &fw);
	mcu->frequency = MCU_FREQ;
	if(gdb_port > 0) {
		mcu->gdb_port = gdb_port;
		avr_gdb_init(mcu);
		if(gdb_stopped) {
			mcu->state = cpu_Stopped;
		}
	}

	avr_irq_register_notify(avr_io_getirq(mcu, AVR_IOCTL_IOPORT_GETIRQ('D'), 3), handle_leds_enable, NULL);
	avr_irq_register_notify(avr_io_getirq(mcu, AVR_IOCTL_IOPORT_GETIRQ('C'), 4), handle_leds_shift, NULL);
	avr_irq_register_notify(avr_io_getirq(mcu, AVR_IOCTL_IOPORT_GETIRQ('C'), 5), handle_leds_store, NULL);

	input_irq = avr_io_getirq(mcu, AVR_IOCTL_UART_GETIRQ('0'), UART_IRQ_INPUT);
	if(drop_rate > 0) {
		srand(time(NULL));
		input_overflow_prob = RAND_MAX / 100 * drop_rate;
		printf("UART overflow probability set to %d %%\n", drop_rate);
	}
	avr_irq_register_notify(avr_io_getirq(mcu, AVR_IOCTL_UART_GETIRQ('0'), UART_IRQ_OUTPUT), handle_uart_output, NULL);
	avr_irq_register_notify(avr_io_getirq(mcu, AVR_IOCTL_UART_GETIRQ('0'), UART_IRQ_OUT_XON), handle_uart_xon, NULL);
	avr_irq_register_notify(avr_io_getirq(mcu, AVR_IOCTL_UART_GETIRQ('0'), UART_IRQ_OUT_XOFF), handle_uart_xoff, NULL);
}

void mcu_start(void) {
	pthread_create(&mcu_thread, NULL, mcu_run, mcu);
}
