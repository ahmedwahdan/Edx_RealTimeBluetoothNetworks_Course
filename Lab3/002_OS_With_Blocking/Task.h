#ifndef TASK_H
#define TASK_H

/********************************************************/
/*										Include														*/
/********************************************************/
#include "stdint.h"

/********************************************************/
/*										Typedef 													*/
/********************************************************/
typedef void (*vTaskPtr)(void);
typedef void (*vPeriodicFunPtr)(void);


/********************************************************/
/*										Structure													*/
/********************************************************/
typedef struct tcb
{
	uint32_t		*TCB_SP;
	struct tcb	*Next_TCB;
	uint32_t		Blocking_State;
}TCB_t;


typedef struct 
{
	uint32_t		Periodicity_Ticks;
	vPeriodicFunPtr	Fun_Ptr;
}Periodic_Fun_t;

/********************************************************/
/*										Prototypes												*/
/********************************************************/
void Tasks_Add(vTaskPtr tasks_Array[]);
void Periodic_Functions_Add(Periodic_Fun_t periodic_Func_Array[]);
#endif /*	TASK_H	*/

