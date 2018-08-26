#include "app.h"
#include "system.h"
#include "task.h"

int main(void)
{
	// Init multitasking
	system_task_init();
	app_tasks_init();
	tasks_init();

	// Pass control to system task
	task_start(SYSTEM_TASK, system_run);
	task_schedule();
}
