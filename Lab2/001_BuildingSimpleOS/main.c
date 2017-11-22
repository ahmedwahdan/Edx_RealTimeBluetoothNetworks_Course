#include <stdint.h>
#include "OS.h"
#include "Task_User_Config.h"
#include "Task.h"
#include "CortexM.h"
void Task1(void);
void Task2(void);
void Task3(void);	

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
	uint32_t  counter2 = 0;
uint32_t   counter3 = 0;
void Task1(void)
{


	while(1)
	{
		/*
		if(counter1 == 1)
		{
			counter1 = 0;
			counter2 = 1;
			GPIOA->DATA ^=(1<<2);
		}
		*/
		counter1++;
	}
}

void Task2(void)
{

	while(1)
	{
		/*
		if(counter2 == 1)
		{
			counter2 = 0;
			counter1 = 1;
			GPIOA->DATA ^=(1<<3);
		}
		*/
		counter2++;
	}
}

void Task3(void)
{
	
	while(1)
	{
		counter3++;
	}
}
