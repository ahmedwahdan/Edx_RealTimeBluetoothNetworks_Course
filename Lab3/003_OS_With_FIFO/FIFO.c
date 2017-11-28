#include "FIFO.h"
#include "Semaphore.h"

#define FIFO_SIZE	(6U)
uint32_t FIFO_Get_Index = 0U;
uint32_t FIFO_Put_Index = 0U;
uint32_t FIFO_Left_Elements = 0U;
uint32_t FIFO_Empty_Space = FIFO_SIZE;
uint32_t FIFO[FIFO_SIZE];
Semaphore Mutex = 1U;

uint32_t FIFO_GET(void)
{
	uint32_t data;
	OS_Wait((Semaphore *)&FIFO_Left_Elements);
	OS_Wait(&Mutex);
	data = FIFO[FIFO_Get_Index++];
	if(FIFO_Get_Index == FIFO_SIZE)
	{
		FIFO_Get_Index = 0;
	}
	OS_Signal(&Mutex);
	OS_Signal((Semaphore *)&FIFO_Empty_Space);
	return data;
}
void FIFO_PUT(uint32_t data)
{
	OS_Wait((Semaphore *)&FIFO_Empty_Space);
	OS_Wait(&Mutex);
	FIFO[FIFO_Put_Index++] = data;
	if(FIFO_Put_Index == FIFO_SIZE)
	{
		FIFO_Put_Index = 0;
	}
	OS_Signal(&Mutex);
	OS_Signal((Semaphore *)&FIFO_Left_Elements);
	
}
