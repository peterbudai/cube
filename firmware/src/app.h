/**
 * @file app.h
 * Common application interface.
 *
 * @copyright (C) 2017 Peter Budai
 */

#ifndef APP_H
#define APP_H

#include "task.h"
#include "system.h"

#define APP_TASK 1
#define APP_STACK_SIZE 128
#define APP_STACK_ADDR (SYSTEM_STACK_ADDR + SYSTEM_STACK_SIZE)
#define APP_RECV_BUFFER_SIZE 512
#define APP_SEND_BUFFER_SIZE 64

/// Number of implemented applications.
#define APP_COUNT 2

/// Pointers to the application entry points.
extern task_func_t apps[];

/// Initializes application list.
void app_tasks_init(void);

/**
 * @name Application forward declarations.
 * Do not call these directly, but via @ref apps.
 */
/// @{

/**
 * This app turns the cube off completely and simply waits for other command.
 * App index is 0.
 */
void app_off(void);

/**
 * General test app that scrolls various letters and symbols, automatically
 * or via USART communications and allows simple testing of all peripherials.
 * App index is 1.
 */
void app_test(void);

/// @}

#endif
