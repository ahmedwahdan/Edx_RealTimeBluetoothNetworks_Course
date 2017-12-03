#include "OS.h"
#include "Task.h"
#include "Task_User_Config.h"
#include "../../Shared/Edx_files/CortexM.h"


void static SYSTick_Timer_Init(void);
void _OS_Start(void);
extern TCB_t	*Run_Task_TCB_Ptr;
extern Periodic_Fun_t g_periodic_Func_Array[PERODIC_FUN_NUMBER];

void OS_Start(void)
{

	// disable global interrupt
	DisableInterrupts();
	// init systick timer
	SYSTick_Timer_Init();
	// enable global interrupt
	// call _OS_Start
	_OS_Start();
}


void SYSTick_Timer_Init(void)
{
	uint32_t sysTick_Timer_Load_Val = 0;
	// disable timer and clear interrupt
	SysTick->CTRL = 0x00U;
	// set timer reload value
	sysTick_Timer_Load_Val = (CPU_CLOCK_HZ / SYS_TICKS_PER_SEC) - 1U;
	#if (sysTick_Timer_Load_Val > 0xFFFFFF)
	#error "SysTick value is not supported"
	#else
	//#error "SysTick value is  supported"
	#endif
	SysTick->LOAD = sysTick_Timer_Load_Val;
	// clear currentvla register
	SysTick->VAL = 0x00U;
	//set SYStick interrupt periority to the lowest priority (To switch only main thread not event thread)
	SYSPRI3 |= (0x7U << 29);
	// enable timer
	SysTick->CTRL = 0x7; /*	Enable timer, Enable interrupt, System bus colck source	*/
}


void OS_Scheduler(void)
{
	uint32_t static Ticks = 0;
	uint8_t index = 0;
	Ticks++;
	for(index = 0; index < PERODIC_FUN_NUMBER; index++)
	{
		if((Ticks % g_periodic_Func_Array[index].Periodicity_Ticks  ) == 0)
		{
			g_periodic_Func_Array[index].Fun_Ptr();
		}
		
	}
	Run_Task_TCB_Ptr = Run_Task_TCB_Ptr->Next_TCB;
}

void OS_Suspend(void)
{
	SysTick->VAL = 0U;
	INTCTRL = (1U<<SYSTICKPEND);
}
