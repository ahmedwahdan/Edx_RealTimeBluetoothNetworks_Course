// os.c
// Runs on LM4F120/TM4C123/MSP432
// Lab 3 starter file.
// Daniel Valvano
// March 24, 2016

#include <stdint.h>
#include "os.h"
#include "../../Shared/Edx_files/CortexM.h"
#include "../../Shared/Edx_files/BSP.h"

// function definitions in osasm.s
void StartOS(void);
typedef void(*vPeriodicThread)(void);

struct tcb{
  int32_t *sp;       // pointer to stack (valid for threads not running
  struct tcb *next;  // linked-list pointer
  uint32_t		Blocking_State;
  uint32_t   Sleep_Counts;
//*FILL THIS IN****
};
typedef struct 
{
	uint32_t		Periodicity_Ticks;
	vPeriodicThread	Fun_Ptr;
}Periodic_Fun_t;
typedef struct tcb tcbType;
tcbType tcbs[NUMTHREADS];
tcbType *RunPt;
int32_t Stacks[NUMTHREADS][STACKSIZE];


// ******** OS_Init ************
// Initialize operating system, disable interrupts
// Initialize OS controlled I/O: periodic interrupt, bus clock as fast as possible
// Initialize OS global variables
// Inputs:  none
// Outputs: none
void static runperiodicevents(void);
void OS_Init(void){
  DisableInterrupts();
  BSP_Clock_InitFastest();// set processor clock to fastest speed
  // perform any initializations needed
	BSP_PeriodicTask_Init(runperiodicevents,1000,5);
}

void SetInitialStack(int i){

  // **Same as Lab 2****
}

//******** OS_AddThreads ***************
// Add six main threads to the scheduler
// Inputs: function pointers to six void/void main threads
// Outputs: 1 if successful, 0 if this thread can not be added
// This function will only be called once, after OS_Init and before OS_Launch
void OS_AddThreads(vTaskPtr tasks_Array[])
{
	uint8_t task_Index = 0;
	for(task_Index = 0; task_Index < NUMTHREADS; task_Index++)									
	{
		tcbs[task_Index].sp = &Stacks[task_Index][STACKSIZE-16];
		tcbs[task_Index].next = &tcbs[task_Index+1];
		tcbs[task_Index].Blocking_State = 0U;
		Stacks[task_Index][STACKSIZE- 1] 	= 0x01000000;	/*	Thumb mode	*/
		Stacks[task_Index][STACKSIZE- 2] 	= (uint32_t)tasks_Array[task_Index];
		Stacks[task_Index][STACKSIZE- 3] 	= 0x03030303;
		Stacks[task_Index][STACKSIZE- 4] 	= 0x04040404;
		Stacks[task_Index][STACKSIZE- 5] 	= 0x05050505;
		Stacks[task_Index][STACKSIZE- 6] 	= 0x06060606;
		Stacks[task_Index][STACKSIZE- 7] 	= 0x07070707;
		Stacks[task_Index][STACKSIZE- 8] 	= 0x08080808;
		Stacks[task_Index][STACKSIZE- 9] 	= 0x09090909;
		Stacks[task_Index][STACKSIZE-10] 	= 0x0A0A0A0A;
		Stacks[task_Index][STACKSIZE-11] 	= 0x0B0B0B0B;
		Stacks[task_Index][STACKSIZE-12] 	= 0x0C0C0C0C;
		Stacks[task_Index][STACKSIZE-13] 	= 0x0D0D0D0D;
		Stacks[task_Index][STACKSIZE-14] 	= 0x0E0E0E0E;
		Stacks[task_Index][STACKSIZE-15] 	= 0x0F0F0F0F;
		Stacks[task_Index][STACKSIZE-16] 	= 0x10101010;
		
		
	}
	tcbs[task_Index-1].next = &tcbs[0];
	RunPt = &tcbs[0];
}

//******** OS_AddPeriodicEventThread ***************
// Add one background periodic event thread
// Typically this function receives the highest priority
// Inputs: pointer to a void/void event thread function
//         period given in units of OS_Launch (Lab 3 this will be msec)
// Outputs: 1 if successful, 0 if this thread cannot be added
// It is assumed that the event threads will run to completion and return
// It is assumed the time to run these event threads is short compared to 1 msec
// These threads cannot spin, block, loop, sleep, or kill
// These threads can call OS_Signal
// In Lab 3 this will be called exactly twice

Periodic_Fun_t periodicThreads_Arr[5];
uint32_t periodicThreads_Numbers = 0;
int OS_AddPeriodicEventThread(void(*thread)(void), uint32_t period){
// ****IMPLEMENT THIS****
	if(periodicThreads_Numbers != 5)
	{
		periodicThreads_Arr[periodicThreads_Numbers].Fun_Ptr = thread;
			periodicThreads_Arr[periodicThreads_Numbers].Periodicity_Ticks = period;
		periodicThreads_Numbers++;
		return 1;
	}
	else
	{
		return 0;
	}


}

void static runperiodicevents(void){
// ****IMPLEMENT THIS****
// **RUN PERIODIC THREADS, DECREMENT SLEEP COUNTERS
	uint32_t static Ticks = 0;
	uint8_t index = 0;
	Ticks++;
	for(index = 0; index < NUMTHREADS; index++)									
	{
		if(tcbs[index].Sleep_Counts != 0)
		{
			tcbs[index].Sleep_Counts --;
		}
		
	}
	
	
	
	for(index = 0; index < periodicThreads_Numbers; index++)
	{
		if((Ticks % periodicThreads_Arr[index].Periodicity_Ticks) == 0)
		{
			periodicThreads_Arr[index].Fun_Ptr();
		}
		
	}


	
}

//******** OS_Launch ***************
// Start the scheduler, enable interrupts
// Inputs: number of clock cycles for each time slice
// Outputs: none (does not return)
// Errors: theTimeSlice must be less than 16,777,216
void OS_Launch(uint32_t theTimeSlice){
  STCTRL = 0;                  // disable SysTick during setup
  STCURRENT = 0;               // any write to current clears it
  SYSPRI3 =(SYSPRI3&0x00FFFFFF)|0xE0000000; // priority 7
  STRELOAD = theTimeSlice - 1; // reload value
  STCTRL = 0x00000007;         // enable, core clock and interrupt arm
  StartOS();                   // start on the first task
}
// runs every ms
void Scheduler(void){ // every time slice
// ROUND ROBIN, skip blocked and sleeping threads
	

	RunPt = RunPt->next;

	while((RunPt->Blocking_State)|| (RunPt->Sleep_Counts))
	{
		RunPt = RunPt->next;
	}
}

//******** OS_Suspend ***************
// Called by main thread to cooperatively suspend operation
// Inputs: none
// Outputs: none
// Will be run again depending on sleep/block status
void OS_Suspend(void){
  STCURRENT = 0;        // any write to current clears it
  INTCTRL = 0x04000000; // trigger SysTick
// next thread gets a full time slice
}

// ******** OS_Sleep ************
// place this thread into a dormant state
// input:  number of msec to sleep
// output: none
// OS_Sleep(0) implements cooperative multitasking
void OS_Sleep(uint32_t sleepTime){
// set sleep parameter in TCB
	RunPt->Sleep_Counts = sleepTime;
	OS_Suspend();
// suspend, stops running
}

// ******** OS_InitSemaphore ************
// Initialize counting semaphore
// Inputs:  pointer to a semaphore
//          initial value of semaphore
// Outputs: none
void OS_InitSemaphore(int32_t *semaPt, int32_t value){
//***IMPLEMENT THIS***
	*semaPt = value;
}

// ******** OS_Wait ************
// Decrement semaphore and block if less than zero
// Lab2 spinlock (does not suspend while spinning)
// Lab3 block if less than zero
// Inputs:  pointer to a counting semaphore
// Outputs: none
void OS_Wait(int32_t *semaPt)
{
//***IMPLEMENT THIS***
	long interrupt_State = 0;
	interrupt_State = StartCritical();
	(*semaPt)--;
	if((*semaPt) < 0)
	{
		RunPt->Blocking_State = (uint32_t )semaPt;
		EndCritical(interrupt_State);
		OS_Suspend();
	}
	EndCritical(interrupt_State);
}

// ******** OS_Signal ************
// Increment semaphore
// Lab2 spinlock
// Lab3 wakeup blocked thread if appropriate
// Inputs:  pointer to a counting semaphore
// Outputs: none
void OS_Signal(int32_t *semaPt)
{
//***IMPLEMENT THIS***
	long interrupt_State = 0;
	tcbType *temp_tcb = 0U;
	interrupt_State = StartCritical();
	/* This check removed to enable the FIFO operation => A modification required*/
	//if((*S)<=0)
	//{
		(*semaPt)++;
		if((*semaPt) <= 0)
		{
			temp_tcb = RunPt->next;
			while(temp_tcb->Blocking_State != (uint32_t)semaPt)
			{
				temp_tcb = temp_tcb->next;
			}
			temp_tcb->Blocking_State = 0U;
			
		
		}
	//}
	
	
	EndCritical(interrupt_State);
}

#define FSIZE 10    // can be any size
uint32_t PutI;      // index of where to put next
uint32_t GetI;      // index of where to get next
uint32_t Fifo[FSIZE];
int32_t CurrentSize;// 0 means FIFO empty, FSIZE means full
uint32_t LostData;  // number of lost pieces of data

// ******** OS_FIFO_Init ************
// Initialize FIFO.  
// One event thread producer, one main thread consumer
// Inputs:  none
// Outputs: none
void OS_FIFO_Init(void){
//***IMPLEMENT THIS***
	PutI = 0;
	GetI = 0;
	LostData = 0;
	CurrentSize = 0;
}

// ******** OS_FIFO_Put ************
// Put an entry in the FIFO.  
// Exactly one event thread puts,
// do not block or spin if full
// Inputs:  data to be stored
// Outputs: 0 if successful, -1 if the FIFO is full
int OS_FIFO_Put(uint32_t data){
//***IMPLEMENT THIS***
	if(CurrentSize!= FSIZE)
	{
		Fifo[PutI++] = data;
		if(PutI == FSIZE)
		{
			PutI = 0;
		}
		 OS_Signal(&CurrentSize);
		  return 0;   // success
	}
	else
	{
		LostData++;
		return -1;
	}
	
	


}

// ******** OS_FIFO_Get ************
// Get an entry from the FIFO.   
// Exactly one main thread get,
// do block if empty
// Inputs:  none
// Outputs: data retrieved
uint32_t OS_FIFO_Get(void){
//***IMPLEMENT THIS***
	uint32_t data;
	OS_Wait(&CurrentSize);
	data = Fifo[GetI++];
	if(GetI == FSIZE)
	{
		GetI = 0;
	}
  return data;
}



