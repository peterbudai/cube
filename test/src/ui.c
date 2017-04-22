#include <string.h>
#include <stdio.h>

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include "io.h"

#define DIM_X 0
#define DIM_ROW 0
#define DIM_Y 1
#define DIM_COL 1
#define DIM_Z 2
#define DIM_LAYER 2
#define DIM_MAX 3

#define LED_SCALE 0.06
float LED_DIM[DIM_MAX] = {0.8, 1, 0.8};
float LED_OFF[DIM_MAX] = {0.6, 1, 0.6};

#define DRAW_FPS 50
uint64_t draw_count = 0;

uint64_t last_time = 0;
uint64_t last_ticks = 0;
uint64_t last_draw = 0;
uint64_t last_uart[2] = {0, 0};
char status_lines[3][512] = {"", "", ""};

int rotate_x = 11, rotate_y = -23;
int drag_x = 0, drag_y = 0;
bool drag_on = false;

static void draw_led(bool state) {
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

static float get_led_origin(int dim, int index) {
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

static void draw_leds() {
	uint8_t led_state[LED_COUNT][LED_COUNT];
	bool led_enabled;
	leds_copy_state(led_state, &led_enabled);

	// The whole led cube takes up the whole 2x2x2 virtual space, origin at center
	for(int x = 0; x < LED_COUNT; ++x) {
		for(int y = 0; y < LED_COUNT; ++y) {
			for(int z = 0; z < LED_COUNT; ++z) {
				glPushMatrix();
				glTranslatef(get_led_origin(DIM_X, x), get_led_origin(DIM_Y, y), get_led_origin(DIM_Z, z));
				glScalef(LED_SCALE, LED_SCALE, LED_SCALE);
				draw_led(led_enabled && (led_state[y][x] & (1 << z)) > 0);
				glPopMatrix();
			}
		}
	}
}

static void draw_ligths() {
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

static void draw_info() {
	for(int i = 0; i < 3; ++i) {
		int len = strlen(status_lines[i]);
		glWindowPos2i(1000 - len * 9, 1000 - 15 * (i + 1));
		for(int j = 0; j < len; ++j) {
			glutBitmapCharacter(GLUT_BITMAP_9_BY_15, status_lines[i][j]);
		}
	}
}

static void draw_callback() {
	glClearColor(31/255.0,79/255.0,111/255.0,1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	draw_info();

	glRotatef(rotate_x, 1, 0, 0);
	glRotatef(rotate_y, 0, 1, 0);
	glScalef(0.5, 0.5, 0.5);
	draw_leds();

	glutSwapBuffers();
	draw_count++;
}

static void mouse_click_callback(int button, int state, int x, int y) {
	if(button == GLUT_LEFT_BUTTON) {
		if(state == GLUT_DOWN) {
			drag_on = true;
			drag_x = x;
			drag_y = y;
		} else {
			drag_on = false;
		}
	}
}

static void mouse_move_callback(int x, int y) {
	if(drag_on) {
		int diffX = x - drag_x;
		if(abs(diffX) > 10) {
			rotate_y += diffX / 10;
			rotate_y %= 360;
			drag_x = x;
			glutPostRedisplay();
		}
		int diffY = y - drag_y;
		if(abs(diffY) > 10) {
			rotate_x += diffY / 10;
			if(rotate_x > 30) {
				rotate_x = 30;
			}
			if(rotate_x < -30) {
				rotate_x = -30;
			}
			drag_y = y;
			glutPostRedisplay();
		}
	}
}

static void timer_callback(int value __attribute__((unused))) {
	uint64_t current_time = glutGet(GLUT_ELAPSED_TIME);
	uint64_t elapsed_time = current_time - last_time;

	leds_dim_down();

	if(elapsed_time >= 1000) {
		uint64_t elapsed_ticks = mcu_ticks - last_ticks;
		uint64_t elapsed_draw = draw_count - last_draw;

		uint64_t current_uart[2];
		uint64_t current_drop[2];
		uart_get_counts(current_uart, current_drop);

		float real = (float)current_time / 1000;
		float virt = (float)mcu_ticks / MCU_FREQ;
		float cps = (float)elapsed_ticks / elapsed_time / 1000;
		float cratio = (float)elapsed_ticks / MCU_FREQ * 100;
		float fps = (float)elapsed_draw / elapsed_time * 1000;
		float fratio = fps / DRAW_FPS * 100;
		float bps[2];

		for(int i = 0; i < 2; ++i) {
			uint64_t elapsed_uart = current_uart[i] - last_uart[i];
			bps[i] = (float)elapsed_uart / elapsed_time * 1000;
			last_uart[i] = current_uart[i];
		}
		last_time = current_time;
		last_ticks = mcu_ticks;
		last_draw = draw_count;

		snprintf(status_lines[0], 512, "Real time: %.1f s, Virtual time: %.1f s, CPU speed: %.2f MHz (%.1f %%)", real, virt, cps, cratio);
		snprintf(status_lines[1], 512, "UART in: %lu B (%.1f Bps), dropped: %lu B, UART out: %lu B (%.1f Bps), dropped: %lu B",
				current_uart[UART_INPUT], bps[UART_INPUT], current_drop[UART_INPUT],
				current_uart[UART_OUTPUT], bps[UART_OUTPUT], current_drop[UART_OUTPUT]);
		snprintf(status_lines[2], 512, "H rotation: %d deg, V rotation: %d deg, Frame rate: %.2f FPS (%.1f %%)", rotate_y, rotate_x, fps, fratio);
	}

	glutPostRedisplay();
	glutTimerFunc(1000 / DRAW_FPS, timer_callback, 0);
}

void ui_init(int* argc, char** argv) {
	glutInit(argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);
	glutInitWindowSize(1000, 1000);
	glutCreateWindow("LED Cube Simulator");
	glutDisplayFunc(draw_callback);
	glutMouseFunc(mouse_click_callback);
	glutMotionFunc(mouse_move_callback);
	glutTimerFunc(1, timer_callback, 0);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_LIGHTING);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, 1, 0.1, 100);
	gluLookAt(0,1,2,0,0,0,0,1,-1);
	draw_ligths();
}

void ui_run(void) {
	glutMainLoop();
}
