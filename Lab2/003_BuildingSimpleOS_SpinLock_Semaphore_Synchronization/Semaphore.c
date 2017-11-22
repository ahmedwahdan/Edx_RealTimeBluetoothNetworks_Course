#include "Semaphore.h"
#include "CortexM.h"

void OS_Wait(Semaphore *S)
{
	long interrupt_State = 0;
	interrupt_State = StartCritical();
	while(*S == 0)
	{
		StartCritical();
		EndCritical(interrupt_State);
	}
	(*S)--;
	EndCritical(interrupt_State);
}

void OS_Signal(Semaphore *S)
{
	long interrupt_State = 0;
	interrupt_State = StartCritical();
	(*S)++;
	EndCritical(interrupt_State);
}


