#ifndef APP_H
#define APP_H

#define APP_COUNT 2

typedef void(*app)(void);
extern app apps[APP_COUNT];

void apps_init(void);

void app_off(void);
void app_test(void);

#endif
