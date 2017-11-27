#include <stdint.h>
#include "OS.h"
#include "Task_User_Config.h"
#include "Task.h"
#include "../00-shared/inc/CortexM.h"
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

	uint32_t  counter1 = 1;
	uint32_t  counter2 = 1;
	uint32_t  counter3 = 1;
 uint16_t test = 0;

void Task1(void)
{


	while(1)
	{
		
	  if(counter1 == 1)
		{
			
			counter1 = 0;
			counter2 = 1;
			GPIOA->DATA ^=(1<<2);
			test++;
			if(test == 10)
			{
				test = 0;
				OS_Signal(&Sem);
			}
		}
		
			
			
		
		
	}
}

void Task2(void)
{

	while(1)
	{
	  
			if(counter2 == 1)
			{
				counter2 = 0;
				counter1 = 1;
				GPIOA->DATA ^=(1<<3);
			}
			
			
		 
		
	}
}

void Task3(void)
{
	
	while(1)
	{
			GPIOF->DATA ^=(1<<1);
		  OS_Wait(&Sem);
	}
}



