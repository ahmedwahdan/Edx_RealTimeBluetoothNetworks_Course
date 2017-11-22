#include "Task.h"
#include "Task_User_Config.h"
#include "stdlib.h"

uint32_t Stack[TASKS_NUMBER][STACK_SIZE];

TCB_t	Task_TCBs[TASKS_NUMBER];

TCB_t	*Run_Task_TCB_Ptr = NULL;

Periodic_Fun_t g_periodic_Func_Array[PERODIC_FUN_NUMBER];

/*
 ___________
|___________|  STACK_SIZE - 17
|____R04____|  STACK_SIZE - 16
|____R05____|  STACK_SIZE - 15
|____R06____|  STACK_SIZE - 14
|____R07____|  STACK_SIZE - 13
|____R08____|  STACK_SIZE - 12
|____R09____|  STACK_SIZE - 11
|____R10____|  STACK_SIZE - 10
|____R11____|  STACK_SIZE - 9
|____R00____|  STACK_SIZE - 8
|____R01____|  STACK_SIZE - 7
|____R02____|  STACK_SIZE - 6
|____R03____|  STACK_SIZE - 5
|____R12____|  STACK_SIZE - 4
|____LR_____|  STACK_SIZE - 3
|____PC_____|  STACK_SIZE - 2
|____PSR____|  STACK_SIZE - 1

*/
void Tasks_Add(vTaskPtr tasks_Array[])
{
	uint8_t task_Index = 0;
	for(task_Index = 0; task_Index < TASKS_NUMBER; task_Index++)									
	{
		Task_TCBs[task_Index].TCB_SP = &Stack[task_Index][STACK_SIZE-16];
		Task_TCBs[task_Index].Next_TCB = &Task_TCBs[task_Index+1];
		Stack[task_Index][STACK_SIZE- 1] 	= 0x01000000;	/*	Thumb mode	*/
		Stack[task_Index][STACK_SIZE- 2] 	= (uint32_t)tasks_Array[task_Index];
		Stack[task_Index][STACK_SIZE- 3] 	= 0x03030303;
		Stack[task_Index][STACK_SIZE- 4] 	= 0x04040404;
		Stack[task_Index][STACK_SIZE- 5] 	= 0x05050505;
		Stack[task_Index][STACK_SIZE- 6] 	= 0x06060606;
		Stack[task_Index][STACK_SIZE- 7] 	= 0x07070707;
		Stack[task_Index][STACK_SIZE- 8] 	= 0x08080808;
		Stack[task_Index][STACK_SIZE- 9] 	= 0x09090909;
		Stack[task_Index][STACK_SIZE-10] 	= 0x0A0A0A0A;
		Stack[task_Index][STACK_SIZE-11] 	= 0x0B0B0B0B;
		Stack[task_Index][STACK_SIZE-12] 	= 0x0C0C0C0C;
		Stack[task_Index][STACK_SIZE-13] 	= 0x0D0D0D0D;
		Stack[task_Index][STACK_SIZE-14] 	= 0x0E0E0E0E;
		Stack[task_Index][STACK_SIZE-15] 	= 0x0F0F0F0F;
		Stack[task_Index][STACK_SIZE-16] 	= 0x10101010;
		
		
	}
	Task_TCBs[task_Index-1].Next_TCB = &Task_TCBs[0];
	Run_Task_TCB_Ptr = &Task_TCBs[0];
}


void Periodic_Functions_Add(Periodic_Fun_t periodic_Func_Array[])
{
	 uint8_t index = 0;
	for(index = 0; index < PERODIC_FUN_NUMBER; index++)
	{
		g_periodic_Func_Array[index] = periodic_Func_Array[index];
	}
}
