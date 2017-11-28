#include <stdint.h>
#include "OS.h"
#include "Task_User_Config.h"
#include "Task.h"
#include "../00-shared/inc/CortexM.h"
#include "Semaphore.h"
#include "FIFO.h"
void Task1(void);
void Task2(void);
void Task3(void);	

uint32_t  counter1 = 1;
uint32_t  counter2 = 1;
uint32_t  counter3 = 1;
uint16_t test = 4;
 void Release_Fun()
 {
	 	GPIOA->DATA ^=(1<<5);
	 counter1 = 1;
 }
int main()
{
	SYSCTL->RCGCGPIO |=(1U<<0);
	//GPIOF->DEN |=(1U<<1);
	GPIOA->DEN |=(1U<<2)|(1U<<3)|(1U<<4)|(1U<<5);
	//GPIOF->DIR |=(1U<<1);
	GPIOA->DIR |=(1U<<2)|(1U<<3)|(1U<<4)|(1U<<5);
  
	vTaskPtr Task_Array[TASKS_NUMBER] = {Task1,Task2,Task3};
	Periodic_Fun_t periodic_fun[] = {{10,Release_Fun}};
	Tasks_Add(Task_Array);
	Periodic_Functions_Add(periodic_fun);
	OS_Start();
}

	

void Task1(void)
{


	while(1)
	{
		
	  if(counter1 == 1)
		{
			
			counter1 = 0;

		  GPIOA->DATA ^=(1<<2);
			test = 0;
			FIFO_PUT(1);
			FIFO_PUT(2);
			FIFO_PUT(3);
			GPIOA->DATA ^=(1<<2);
	
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
				counter3 = 1;
				GPIOA->DATA ^=(1<<3);
					(void)FIFO_GET();
			}
			
			
		 
		
	}
}

void Task3(void)
{
	
	while(1)
	{
		if(counter3 == 1)
		{
			counter3 = 0;
			counter2 = 1;
			GPIOA->DATA ^=(1<<4);
		  (void)FIFO_GET();
		}
			
	}
}



