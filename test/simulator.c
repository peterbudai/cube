#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <pthread.h>

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include <simavr/avr_ioport.h>
#include <simavr/sim_avr.h>
#include <simavr/sim_elf.h>
#include <simavr/sim_gdb.h>

// Constants
#define DIM_X 0
#define DIM_ROW 0
#define DIM_Y 1
#define DIM_COL 1
#define DIM_Z 2
#define DIM_LAYER 2
#define DIM_MAX 3

#define LED_COUNT 8
#define LED_SCALE 0.06
float LED_DIM[DIM_MAX] = {0.8, 1, 0.8};
float LED_OFF[DIM_MAX] = {0.6, 1, 0.6};

#define PORTB 0x25
#define PORTC 0x28
#define PORTD 0x2B

// Global variables
uint8_t row_reg[LED_COUNT];
uint8_t row_latch[LED_COUNT][LED_COUNT];
int enabled;

int rotateX = 11, rotateY = -23;
int dragX, dragY;
bool dragging = false;

avr_t* avr;
pthread_t cpu_thread;
pthread_mutex_t io_mutex;
unsigned long cpu_ticks = 0;
unsigned long draw_count = 0;
char status[512] = {'\0'};

void drawLed(bool state) {
	// Red plastic
	float mat[4] = {0, 0, 0, 0};
	if(state) {
		mat[0] = 1.0; mat[1] = 0.35; mat[2] = 0; mat[3] = 0.95;
		glMaterialfv(GL_FRONT, GL_AMBIENT, mat);
		mat[0] = 1.0; mat[1] = 0.35; mat[2] = 0; mat[3] = 0.95;
		glMaterialfv(GL_FRONT, GL_DIFFUSE, mat);
		mat[0] = 1.0; mat[1] = 0.35; mat[2] = 0; mat[3] = 1;
		glMaterialfv(GL_FRONT, GL_EMISSION, mat);
		mat[0] = 1.0; mat[1] = 0.35; mat[2] = 0; mat[3] = 0.95;
		glMaterialfv(GL_FRONT, GL_SPECULAR, mat);
	} else {
		mat[0] = 0.9; mat[1] = 0.9; mat[2] = 0.9; mat[3] = 0.4;
		glMaterialfv(GL_FRONT, GL_AMBIENT, mat);
		mat[0] = 0.9; mat[1] = 0.9; mat[2] = 0.9; mat[3] = 0.4;
		glMaterialfv(GL_FRONT, GL_DIFFUSE, mat);
		mat[0] = 0.7; mat[1] = 0.6; mat[2] = 0.6; mat[3] = 0.4;
		glMaterialfv(GL_FRONT, GL_SPECULAR, mat);
	}
	glMaterialf(GL_FRONT, GL_SHININESS, 32);

	// Led's bottom is a 0.8 diameter cylinder at y=0, its tip is at 0,1,0
	glScalef(1/1.5, 1/1.5, 1/1.5);
	glRotatef(90, -1, 0, 0);
	glTranslatef(0, 0, 0.5);

	GLUquadric* quad = gluNewQuadric();
	glTranslatef(0, 0, 0.5);
	gluSphere(quad, 0.5, 12, 12);
	glTranslatef(0, 0, -0.75);
	gluCylinder(quad, 0.5, 0.5, 0.75, 12, 4);
	gluDisk(quad, 0.5, 0.6, 12, 1);
	glTranslatef(0, 0, -0.25);
	gluCylinder(quad, 0.6, 0.6, 0.25, 12, 4);
	glRotatef(180, 1, 0, 0);
	gluDisk(quad, 0, 0.6, 12, 1);
	gluDeleteQuadric(quad);
}

float getLedOrigin(int dim, int index) {
	// Space occupied by led heads
	float headSpace = LED_DIM[dim] * LED_SCALE * LED_COUNT;
	// Space between led heads
	float emptySpace = (2.0 - headSpace) / (LED_COUNT - 1.0);
	// The center of the first led, assuming it is at the edge
	float firstOffset = (1.0 - LED_OFF[dim]) * LED_SCALE;
	// The distance between the center of two leds
	float centerOffset = (LED_DIM[dim] / 2.0 * 2.0) * LED_SCALE + emptySpace;
	return firstOffset + centerOffset * (float)index - 1.0;
}

void drawLeds() {
	pthread_mutex_lock(&io_mutex);
	uint8_t led_state[LED_COUNT][LED_COUNT];
	memcpy(led_state, row_latch, sizeof(row_latch));
	pthread_mutex_unlock(&io_mutex);

	// The whole led cube takes up the whole 2x2x2 virtual space, origin at center
	for(int x = 0; x < LED_COUNT; ++x) {
		for(int y = 0; y < LED_COUNT; ++y) {
			for(int z = 0; z < LED_COUNT; ++z) {
				glPushMatrix();
				glTranslatef(getLedOrigin(DIM_X, x), getLedOrigin(DIM_Y, y), getLedOrigin(DIM_Z, z));
				glScalef(LED_SCALE, LED_SCALE, LED_SCALE);
				drawLed(enabled > 0 && (led_state[y][x] & (1 << z)) > 0);
				glPopMatrix();
			}
		}
	}
}

void drawLigths() {
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	float lgt[4] = {0, 0, 0, 1};
	lgt[0] = 0.1; lgt[1] = 0.1; lgt[2] = 0.1; lgt[3] = 1;
	glLightfv(GL_LIGHT0, GL_AMBIENT, lgt);
	lgt[0] = 0.5; lgt[1] = 0.5; lgt[2] = 0.5; lgt[3] = 1;
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lgt);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lgt);
	lgt[0] = 1.5; lgt[1] = -1; lgt[2] = 0.5; lgt[3] = 0;
	glLightfv(GL_LIGHT0, GL_POSITION, lgt);
	glEnable(GL_LIGHT0);
}

void drawInfo() {
	int len = strlen(status);
	glWindowPos2i(1000 - len * 9, 1000 - 15);
	for(int i = 0; i < len; ++i) {
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, status[i]);
	}
}

void onDisplay() {
	glClearColor(31/255.0,79/255.0,111/255.0,1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	drawInfo();

	glRotatef(rotateX, 1, 0, 0);
	glRotatef(rotateY, 0, 1, 0);
	glScalef(0.5, 0.5, 0.5);
	drawLeds();

	glutSwapBuffers();
	draw_count++;
}

void onMouse(int button, int state, int x, int y) {
	if(button == GLUT_LEFT_BUTTON) {
		if(state == GLUT_DOWN) {
			dragging = true;
			dragX = x;
			dragY = y;
		} else {
			dragging = false;
		}
	}
}

void onMotion(int x, int y) {
	if(dragging) {
		int diffX = x - dragX;
		if(abs(diffX) > 10) {
			rotateY += diffX / 10;
			rotateY %= 360;
			dragX = x;
			glutPostRedisplay();
		}
		int diffY = y - dragY;
		if(abs(diffY) > 10) {
			rotateX += diffY / 10;
			if(rotateX > 30) {
				rotateX = 30;
			}
			if(rotateX < -30) {
				rotateX = -30;
			}
			dragY = y;
			glutPostRedisplay();
		}
	}
}

void onTimer(int value __attribute__((unused))) {
	static unsigned long last_time = 0;
	static unsigned long last_ticks = 0;
	static unsigned long last_draw = 0;

	unsigned long cur_time = glutGet(GLUT_ELAPSED_TIME);
	unsigned long elapsed_time = cur_time - last_time;

	pthread_mutex_lock(&io_mutex);
	if(enabled > 0) {
		enabled--;
	}

	if(elapsed_time >= 1000) {
		unsigned long elapsed_ticks = cpu_ticks - last_ticks;
		unsigned long elapsed_draw = draw_count - last_draw;

		float real = (float)cur_time / 1000;
		float virt = (float)cpu_ticks / MCU_FREQ;
		float cps = (float)elapsed_ticks / elapsed_time / 1000;
		float ratio = (float)elapsed_ticks / MCU_FREQ * 100;
		float fps = (float)elapsed_draw / elapsed_time * 1000;
		sprintf(status, "Real: %.1f s, Virt: %.1f s, CPU: %.2f MHz (%.1f %%), Sim: %.2f FPS", real, virt, cps, ratio, fps);

		last_time = cur_time;
		last_ticks = cpu_ticks;
		last_draw = draw_count;
	}
	pthread_mutex_unlock(&io_mutex);

	glutPostRedisplay();
	glutTimerFunc(20, onTimer, 0);
}

void* onIdle(void* args __attribute__((unused))) {
	unsigned long real_ticks = 0;
	while(true) {
		if(real_ticks++ % 16384 == 0) {
			pthread_mutex_lock(&io_mutex);
			cpu_ticks = real_ticks;
			pthread_mutex_unlock(&io_mutex);
		}

		avr_run(avr);
	}
	return NULL;
}

void onEnable(struct avr_irq_t* irq __attribute__((unused)), uint32_t value, void* param __attribute__((unused))) {
	if(value == 0) {
		pthread_mutex_lock(&io_mutex);
		enabled = 8;
		pthread_mutex_unlock(&io_mutex);
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
		pthread_mutex_lock(&io_mutex);
		uint8_t layer = avr->data[PORTB] & 0x07;
		for(int i = 0; i < 8; ++i) {
			row_latch[layer][i] = row_reg[i];
		}
		pthread_mutex_unlock(&io_mutex);
	}
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);
	glutInitWindowSize(1000, 1000);
	glutCreateWindow("LED Cube Simulator");
	glutDisplayFunc(onDisplay);
	glutMouseFunc(onMouse);
	glutMotionFunc(onMotion);
	glutTimerFunc(1, onTimer, 0);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_LIGHTING);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, 1, 0.1, 100);
	gluLookAt(0,1,2,0,0,0,0,1,-1);
	drawLigths();

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

	pthread_mutex_init(&io_mutex, NULL);
	pthread_create(&cpu_thread, NULL, onIdle, NULL);

	glutMainLoop();
	return 0;
}
