#include "Semaphore.h"
#include "../../Shared/Edx_files/CortexM.h"
#include "Task.h"
#include "OS.h"
extern TCB_t	*Run_Task_TCB_Ptr;
void OS_Wait(Semaphore *S)
{
	long interrupt_State = 0;
	interrupt_State = StartCritical();
	(*S)--;
	if((*S) < 0)
	{
		Run_Task_TCB_Ptr->Blocking_State = (uint32_t )S;
		EndCritical(interrupt_State);
		OS_Suspend();
	}
	EndCritical(interrupt_State);
}

void OS_Signal(Semaphore *S)
{
	long interrupt_State = 0;
	TCB_t *temp_tcb = 0U;
	interrupt_State = StartCritical();
	/* This check removed to enable the FIFO operation => A modification required*/
	//if((*S)<=0)
	//{
		(*S)++;
		if((*S) <= 0)
		{
			temp_tcb = Run_Task_TCB_Ptr->Next_TCB;
			while(temp_tcb->Blocking_State != (uint32_t)S)
			{
				temp_tcb = temp_tcb->Next_TCB;
			}
			temp_tcb->Blocking_State = 0U;
			
		
		}
	//}
	
	
	EndCritical(interrupt_State);
}


