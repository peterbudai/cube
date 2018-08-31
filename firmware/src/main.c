#include "app.h"
#include "system.h"
#include "task.h"

/// The entry point of the application.
int main(void)
{
	// RAM layout
	//
	// Address  Symbolic name        Contents
	// -------  -------------        --------
	// 0x8FF    RAMEND                      |                           |
	// ...                                  | idle stack                |
	// 0x8C0    RAMEND - IDLE_STACK_SIZE    |                           |
	// 0x8BF	SYSTEM_STACK_START                      |               |
	// ...			                                    | system stack  | boot stack
	// 0x860	SYSTEM_STACK_START - SYSTEM_STACK_SIZE  |               |
	// 0x85F	APP_STACK_START             |                           |
	// ...                                  | app stack                 |
	// 0x???    CPU_STACK_END               |                           |
	// 0x???    CPU_STACK_END - 1    |
	// ...                           | global variables
	// 0x100    RAMSTART             |

	// Init multitasking
	system_task_init();
	app_tasks_init();

	// Pass control to system task
	task_add(SYSTEM_TASK, system_run);
	task_add(APP_TASK, apps[0]);
	tasks_start();

	// Idle task will continue here
	for(;;) {
		cpu_sleep();
	}
}
