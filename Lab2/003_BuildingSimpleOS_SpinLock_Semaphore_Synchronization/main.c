#include <stdint.h>
#include "OS.h"
#include "Task_User_Config.h"
#include "Task.h"
#include "CortexM.h"
#include "Semaphore.h"
void Task1(void);
void Task2(void);
void Task3(void);	
Semaphore Sem = 0;

int main()
{
	SYSCTL->RCGCGPIO |=(1U<<5)|(1U<<0);
	GPIOF->DEN |=(1U<<1);
	GPIOA->DEN |=(1U<<2)|(1U<<3);
	GPIOF->DIR |=(1U<<1);
	GPIOA->DIR |=(1U<<2)|(1U<<3);
  
	vTaskPtr Task_Array[TASKS_NUMBER] = {Task1,Task2,Task3};
	Tasks_Add(Task_Array);
	OS_Start();
}

	uint32_t  counter1 = 0;
	uint32_t  increment = 1;
	


void Task1(void)
{


	while(1)
	{
		increment = 1;
	
		if(counter1 == 2)
		{
			counter1 = 0;
			OS_Signal(&Sem);
			GPIOA->DATA ^=(1<<2);
		}
		
		
	}
}

void Task2(void)
{

	while(1)
	{
	  OS_Wait(&Sem);
		
			GPIOA->DATA ^=(1<<3);
		
	}
}

void Task3(void)
{
	
	while(1)
	{
		if(increment == 1)
		{
			counter1++;
			increment = 0;
		};
	}
}



