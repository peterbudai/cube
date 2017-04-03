#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <pthread.h>

#include <simavr/avr_ioport.h>
#include <simavr/sim_avr.h>
#include <simavr/sim_elf.h>
#include <simavr/sim_gdb.h>

#include "io.h"
#include "ui.h"

// Constants
#define PORTB 0x25
#define PORTC 0x28
#define PORTD 0x2B

// Global variables
uint8_t row_reg[LED_COUNT];

avr_t* avr;
pthread_t cpu_thread;

void* onIdle(void* args __attribute__((unused))) {
	unsigned long real_ticks = 0;
	while(true) {
		if(real_ticks++ % 16384 == 0) {
			cpu_ticks = real_ticks;
		}

		avr_run(avr);
	}
	return NULL;
}

void onEnable(struct avr_irq_t* irq __attribute__((unused)), uint32_t value, void* param __attribute__((unused))) {
	if(value == 0) {
		leds_dim_up();
	}
}

void onShift(struct avr_irq_t* irq __attribute__((unused)), uint32_t value, void* param __attribute__((unused))) {
	if(value) {
		uint8_t row_data = (avr->data[PORTD] & 0xF0) | (avr->data[PORTC] & 0x0F);
		for(int i = 0; i < 8; ++i) {
			row_reg[i] = (row_reg[i] << 1) | ((row_data >> i) & 0x01);
		}
	}
}

void onStore(struct avr_irq_t* irq __attribute__((unused)), uint32_t value, void* param __attribute__((unused))) {
	if(value) {
		uint8_t layer = avr->data[PORTB] & 0x07;
		leds_set_layer(layer, row_reg);
	}
}

int main(int argc, char** argv) {
	ui_init(&argc, argv);
	leds_init();

	elf_firmware_t fw;
	elf_read_firmware("../firmware/out/firmware.elf", &fw);
	avr = avr_make_mcu_by_name(MCU_NAME);
	avr_init(avr);
	avr_load_firmware(avr, &fw);
	avr->frequency = MCU_FREQ;

	for(int i = 1; i < argc; ++i) {
		if(strcmp(argv[i], "-g") == 0) {
			avr->gdb_port = 1234;
			if(i + 1 < argc && strcmp(argv[i + 1], "-p") == 0) {
				avr->state = cpu_Stopped;
			}
			avr_gdb_init(avr);
		}
	}

	avr_irq_register_notify(avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), 3), onEnable, NULL);
	avr_irq_register_notify(avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('C'), 4), onShift, NULL);
	avr_irq_register_notify(avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('C'), 5), onStore, NULL);

	pthread_create(&cpu_thread, NULL, onIdle, NULL);

	ui_run();
	return 0;
}
