#include "io.h"

#include <string.h>

uint8_t leds_state[LED_COUNT][LED_COUNT];
int leds_enabled;
pthread_mutex_t leds_mutex;

void leds_init(void) {
	leds_enabled = 0;
	memset(leds_state, 0, sizeof(leds_state));
pthread_mutex_init(&leds_mutex, NULL);
}

void leds_copy_state(uint8_t state[][LED_COUNT], bool* enabled) {
	pthread_mutex_lock(&leds_mutex);
	memcpy(state, leds_state, sizeof(leds_state));
	*enabled = leds_enabled > 0;
	pthread_mutex_unlock(&leds_mutex);
}

void leds_set_layer(uint8_t layer, uint8_t value[]) {
	pthread_mutex_lock(&leds_mutex);
	for(size_t i = 0; i < LED_COUNT; ++i) {
		leds_state[layer][i] = value[i];
	}
	pthread_mutex_unlock(&leds_mutex);
}

void leds_dim_up(void) {
	pthread_mutex_lock(&leds_mutex);
	leds_enabled = 8;
	pthread_mutex_unlock(&leds_mutex);
}

void leds_dim_down(void) {
	pthread_mutex_lock(&leds_mutex);
	if(leds_enabled > 0) {
		leds_enabled--;
	}
	pthread_mutex_unlock(&leds_mutex);
}
