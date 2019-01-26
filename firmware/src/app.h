/**
 * @file app.h
 * Common application interface.
 *
 * @copyright (C) 2017 Peter Budai
 */
#pragma once

#include "cpu.h"
#include "system.h"
#include "task.h"

/// Application task properties
#define APP_TASK 1
#define APP_STACK_START (SYSTEM_STACK_START - SYSTEM_STACK_SIZE)
#define APP_STACK_SIZE (APP_STACK_START - CPU_STACK_END + 1)
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
 * This app puts the cube into standby mode by completely turning the cube off,
 * and waiting for other app to start.
 * App index is 0.
 */
void app_standby(void);

/**
 * General test app that scrolls various letters and symbols, automatically
 * or via USART communications and allows simple testing of all peripherials.
 * App index is 1.
 */
void app_test(void);

/// @}
