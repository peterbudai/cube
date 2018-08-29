#include "app.h"
#include "system.h"
#include "task.h"

/// The entry point of the application.
int main(void)
{
	// Init multitasking
	system_task_init();
	app_tasks_init();

	// Pass control to system task
	task_start(SYSTEM_TASK, system_run);
	tasks_run();
}
