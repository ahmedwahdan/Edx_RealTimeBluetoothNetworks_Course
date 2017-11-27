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
=====================================================
Fire the systick interrupt:

Register 67: Interrupt Control and State (INTCTRL), offset 0xD04
Note: This register can only be accessed from privileged mode.
The INCTRL register provides a set-pending bit for the NMI exception, and set-pending and
clear-pending bits for the PendSV and SysTick exceptions. In addition, bits in this register indicate
the exception number of the exception being processed, whether there are preempted active
exceptions, the exception number of the highest priority pending exception, and whether any interrupts
are pending.			

26 PENDSTSET RW  0   SysTick Set Pending
