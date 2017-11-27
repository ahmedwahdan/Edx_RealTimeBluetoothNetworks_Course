Chapter3 objectives:
1- Cooperation using suspend
2- Blocking semaphore
3- Data flow with first in first out queues
4- Thread sleeping
5- Periodic interrupts to manage periodic tasks


==================================================
Semaphores:
1- Effeciency
	a- Spinlock => wasteful
	b- Suspend	=> cooperation, more effecient but waste time in switching each time suspend function is called
	c- Blocking	=> very effecient, the task will be blocked until the resource is available

2- Faireness
		- The time a task will wait for a semaphore must be bounded

3- Priority	
	- Blocking semaphore:
		- If more than one task is blocked on a resorce, the highest priority task will get 
			the resouce once it's available

