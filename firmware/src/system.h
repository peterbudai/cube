#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdbool.h>

#include "usart.h"

void system_handle_timer(void);
bool system_handle_usart_input(usart_message_t* message);
bool system_handle_usart_output(usart_message_t* message);
void system_handle_events(void);

bool system_run(void);

#endif
