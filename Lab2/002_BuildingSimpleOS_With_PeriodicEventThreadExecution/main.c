#include <stdint.h>
#include "OS.h"
#include "Task_User_Config.h"
#include "Task.h"
#include "CortexM.h"
void Task1(void);
void Task2(void);
void Task3(void);	
void Periodic_Function_2ms(void);
void Periodic_Function_3ms(void);

int main()
{
	SYSCTL->RCGCGPIO |=(1U<<5)|(1U<<0);
	GPIOF->DEN |=(1U<<1);
	GPIOA->DEN |=(1U<<2)|(1U<<3);
	GPIOF->DIR |=(1U<<1);
	GPIOA->DIR |=(1U<<2)|(1U<<3);

	vTaskPtr Task_Array[TASKS_NUMBER] = {Task1,Task2,Task3};
	Periodic_Fun_t Periodic_Fun_Array[PERODIC_FUN_NUMBER] = 
	{	{5U,Periodic_Function_2ms},
		{1U,Periodic_Function_3ms}
	};
	Tasks_Add(Task_Array);
	Periodic_Functions_Add(Periodic_Fun_Array);
	OS_Start();
}

	uint32_t  counter1 = 0;
	uint32_t  counter2 = 0;
	uint32_t   counter3 = 0;
	uint32_t  Func1_counter = 0;
	uint32_t  Func2_counter = 0;

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


void Periodic_Function_2ms(void)
{
	uint16_t loop = 20;
	while(loop > 0 )
	{
		loop--;
		__asm("NOP");
	}
	Func1_counter++;
	
}

void Periodic_Function_3ms(void)
{
	Func2_counter++;
	GPIOA->DATA ^=(1<<2);
}
