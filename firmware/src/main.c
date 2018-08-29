#include "app.h"
#include "system.h"
#include "task.h"

/// The entry point of the application.
int main(void)
{
	// RAM layout
	//
	// Symbolic name               Description
	// -------------               -----------
	// RAMEND                      |
	// ...                         | system stack
	// RAMEND - SYSTEM_STACK_SIZE  |
	// APP_STACK_START                                       |
	// ...                                                   |
	// CPU_STACK_END + CPU_STACK_INIT_SIZE  |                | app stack
	// ...                                  | initial stack  |
	// CPU_STACK_END                        |                |
	// CPU_STACK_END - 1           |
	// ...                         | global variables
	// RAMSTART                    |

	// Init multitasking
	system_task_init();
	app_tasks_init();

	// Pass control to system task
	task_start(SYSTEM_TASK, system_run);
	tasks_run();
}
