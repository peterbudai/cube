#include "io.h"

#include <string.h>

volatile uint64_t cpu_ticks;
leds_t leds;

void leds_init(void) {
	cpu_ticks = 0;
	leds.enabled = 0;
	memset(leds.state, 0, sizeof(leds));
	pthread_mutex_init(&leds.mutex, NULL);
}

void leds_copy_state(uint8_t state[][LED_COUNT], bool* enabled) {
	pthread_mutex_lock(&leds.mutex);
	memcpy(state, leds.state, sizeof(leds.state));
	*enabled = leds.enabled > 0;
	pthread_mutex_unlock(&leds.mutex);
}

void leds_set_layer(uint8_t layer, uint8_t value[]) {
	pthread_mutex_lock(&leds.mutex);
	for(size_t i = 0; i < LED_COUNT; ++i) {
		leds.state[layer][i] = value[i];
	}
	pthread_mutex_unlock(&leds.mutex);
}

void leds_dim_up(void) {
	pthread_mutex_lock(&leds.mutex);
	leds.enabled = 8;
	pthread_mutex_unlock(&leds.mutex);
}

void leds_dim_down(void) {
	pthread_mutex_lock(&leds.mutex);
	if(leds.enabled > 0) {
		leds.enabled--;
	}
	pthread_mutex_unlock(&leds.mutex);
}
