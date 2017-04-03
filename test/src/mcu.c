#include "mcu.h"

#include <stdbool.h>
#include <string.h>

#include <pthread.h>

#include <simavr/avr_ioport.h>
#include <simavr/sim_avr.h>
#include <simavr/sim_elf.h>
#include <simavr/sim_gdb.h>

#include "io.h"

// Constants
#define PORTB 0x25
#define PORTC 0x28
#define PORTD 0x2B

// Global variables
volatile uint64_t mcu_ticks = 0;

avr_t* mcu = NULL;
pthread_t mcu_thread;
uint8_t shift_reg[LED_COUNT];

static void* mcu_run(void* args) {
	uint64_t real_ticks = 0;
	while(true) {
		if(real_ticks++ % 16384 == 0) {
			mcu_ticks = real_ticks;
		}

		avr_run((avr_t*)args);
	}
	return NULL;
}

static void handle_enable(struct avr_irq_t* irq __attribute__((unused)), uint32_t value, void* param __attribute__((unused))) {
	if(value == 0) {
		leds_dim_up();
	}
}

static void handle_shift(struct avr_irq_t* irq __attribute__((unused)), uint32_t value, void* param) {
	if(value) {
		avr_t* avr = (avr_t*)param;
		uint8_t row_data = (avr->data[PORTD] & 0xF0) | (avr->data[PORTC] & 0x0F);
		for(int i = 0; i < 8; ++i) {
			shift_reg[i] = (shift_reg[i] << 1) | ((row_data >> i) & 0x01);
		}
	}
}

static void handle_store(struct avr_irq_t* irq __attribute__((unused)), uint32_t value, void* param) {
	if(value) {
		uint8_t layer = ((avr_t*)param)->data[PORTB] & 0x07;
		leds_set_layer(layer, shift_reg);
	}
}


void mcu_init(int argc, char** argv) {
	memset(shift_reg, 0, sizeof(shift_reg));

	elf_firmware_t fw;
	elf_read_firmware("../firmware/out/firmware.elf", &fw);

	mcu = avr_make_mcu_by_name(MCU_NAME);
	avr_init(mcu);
	avr_load_firmware(mcu, &fw);
	mcu->frequency = MCU_FREQ;

	for(int i = 1; i < argc; ++i) {
		if(strcmp(argv[i], "-g") == 0) {
			mcu->gdb_port = 1234;
			if(i + 1 < argc && strcmp(argv[i + 1], "-p") == 0) {
				mcu->state = cpu_Stopped;
			}
			avr_gdb_init(mcu);
		}
	}

	avr_irq_register_notify(avr_io_getirq(mcu, AVR_IOCTL_IOPORT_GETIRQ('D'), 3), handle_enable, mcu);
	avr_irq_register_notify(avr_io_getirq(mcu, AVR_IOCTL_IOPORT_GETIRQ('C'), 4), handle_shift, mcu);
	avr_irq_register_notify(avr_io_getirq(mcu, AVR_IOCTL_IOPORT_GETIRQ('C'), 5), handle_store, mcu);
}

void mcu_start(void) {
	pthread_create(&mcu_thread, NULL, mcu_run, mcu);
}
