// os.c
// Runs on LM4F120/TM4C123/MSP432
// Lab 2 starter file.
// Daniel Valvano
// February 20, 2016

#include <stdint.h>
#include "os.h"
#include "CortexM.h"
#include "BSP.h"

// function definitions in osasm.s
void StartOS(void);
void Task0(void);
void Task1(void);
tcbType tcbs[NUMTHREADS];
tcbType *RunPt;
int32_t Stacks[NUMTHREADS][STACKSIZE];


// ******** OS_Init ************
// Initialize operating system, disable interrupts
// Initialize OS controlled I/O: systick, bus clock as fast as possible
// Initialize OS global variables
// Inputs:  none
// Outputs: none
void OS_Init(void){
  DisableInterrupts();
  BSP_Clock_InitFastest();// set processor clock to fastest speed
  // initialize any global variables as needed
  //***YOU IMPLEMENT THIS FUNCTION*****

}

void SetInitialStack(int i){
  //***YOU IMPLEMENT THIS FUNCTION*****

}

//******** OS_AddThreads ***************
// Add four main threads to the scheduler
// Inputs: function pointers to four void/void main threads
// Outputs: 1 if successful, 0 if this thread can not be added
// This function will only be called once, after OS_Init and before OS_Launch
int OS_AddThreads(void(*thread0)(void),
                  void(*thread1)(void),
                  void(*thread2)(void),
                  void(*thread3)(void)){
										
uint8_t index = 0;
	for(index = 0; index < NUMTHREADS; index++)
	{
		tcbs[index].sp = &Stacks[index][STACKSIZE - 16];
		Stacks[index][STACKSIZE - 1] = 0x01000000;
		
	}
	tcbs[0].next = &tcbs[1];
	tcbs[1].next = &tcbs[2];
	tcbs[2].next = &tcbs[3];
	tcbs[3].next = &tcbs[0];
	Stacks[0][STACKSIZE - 2] = (uint32_t)thread0;
	Stacks[1][STACKSIZE - 2] = (uint32_t)thread1;
	Stacks[2][STACKSIZE - 2] = (uint32_t)thread2;
	Stacks[3][STACKSIZE - 2] = (uint32_t)thread3;
	
// initialize RunPt
		RunPt = tcbs;							 										
// initialize TCB circular list
// initialize RunPt
// initialize four stacks, including initial PC
  //***YOU IMPLEMENT THIS FUNCTION*****

  return 1;               // successful
}

//******** OS_AddThreads3 ***************
// add three foregound threads to the scheduler
// This is needed during debugging and not part of final solution
// Inputs: three pointers to a void/void foreground tasks
// Outputs: 1 if successful, 0 if this thread can not be added
int OS_AddThreads3(void(*task0)(void),
                 void(*task1)(void),
                 void(*task2)(void)){ 
// initialize TCB circular list (same as RTOS project)
	uint8_t index = 0;
	for(index = 0; index < NUMTHREADS; index++)
	{
		tcbs[index].sp = &Stacks[index][STACKSIZE - 16];
		Stacks[index][STACKSIZE - 1] = 0x01000000;
		
	}
	tcbs[0].next = &tcbs[1];
	tcbs[1].next = &tcbs[2];
	tcbs[2].next = &tcbs[0];
	Stacks[0][STACKSIZE - 2] = (uint32_t)task0;
	Stacks[1][STACKSIZE - 2] = (uint32_t)task1;
	Stacks[2][STACKSIZE - 2] = (uint32_t)task2;
// initialize RunPt
		RunPt = tcbs;							 
// initialize four stacks, including initial PC
									 
  //***YOU IMPLEMENT THIS FUNCTION*****

  return 1;               // successful
}
                 
//******** OS_AddPeriodicEventThreads ***************
// Add two background periodic event threads
// Typically this function receives the highest priority
// Inputs: pointers to a void/void event thread function2
//         periods given in units of OS_Launch (Lab 2 this will be msec)
// Outputs: 1 if successful, 0 if this thread cannot be added
// It is assumed that the event threads will run to completion and return
// It is assumed the time to run these event threads is short compared to 1 msec
// These threads cannot spin, block, loop, sleep, or kill
// These threads can call OS_Signal
int OS_AddPeriodicEventThreads(void(*thread1)(void), uint32_t period1,
  void(*thread2)(void), uint32_t period2){
  //***YOU IMPLEMENT THIS FUNCTION*****

  return 1;
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
  // run any periodic event threads if needed
  // implement round robin scheduler, update RunPt
  //***YOU IMPLEMENT THIS FUNCTION*****
	static uint32_t time = 0;
	time++;
		Task0();
	if(time == 100)
	{
		Task1();
		time = 0;
		
	}
	RunPt = RunPt->next;
	

}

// ******** OS_InitSemaphore ************
// Initialize counting semaphore
// Inputs:  pointer to a semaphore
//          initial value of semaphore
// Outputs: none
void OS_InitSemaphore(int32_t *semaPt, int32_t value){
  //***YOU IMPLEMENT THIS FUNCTION*****
	*semaPt = value;

}

// ******** OS_Wait ************
// Decrement semaphore
// Lab2 spinlock (does not suspend while spinning)
// Lab3 block if less than zero
// Inputs:  pointer to a counting semaphore
// Outputs: none
void OS_Wait(int32_t *semaPt){
	uint8_t PRIMASK_Status = StartCritical();
	while(*semaPt == 0)
	{
		EndCritical(PRIMASK_Status);
		PRIMASK_Status = StartCritical();
	}
	(*semaPt)--;
	EndCritical(PRIMASK_Status);
}

// ******** OS_Signal ************
// Increment semaphore
// Lab2 spinlock
// Lab3 wakeup blocked thread if appropriate
// Inputs:  pointer to a counting semaphore
// Outputs: none
void OS_Signal(int32_t *semaPt){
//***YOU IMPLEMENT THIS FUNCTION*****
	uint8_t PRIMASK_Status = StartCritical();
	(*semaPt)++;
	EndCritical(PRIMASK_Status);
}



int32_t mailbox_sem;
int32_t mailbox_data;
// ******** OS_MailBox_Init ************
// Initialize communication channel
// Producer is an event thread, consumer is a main thread
// Inputs:  none
// Outputs: none
void OS_MailBox_Init(void){
  // include data field and semaphore
  //***YOU IMPLEMENT THIS FUNCTION*****
	mailbox_sem = 0;
	mailbox_data = 0;
}

// ******** OS_MailBox_Send ************
// Enter data into the MailBox, do not spin/block if full
// Use semaphore to synchronize with OS_MailBox_Recv
// Inputs:  data to be sent
// Outputs: none
// Errors: data lost if MailBox already has data
void OS_MailBox_Send(uint32_t data){
  //***YOU IMPLEMENT THIS FUNCTION*****
	uint8_t PRIMASK_Status = StartCritical();
	if(mailbox_sem == 0)
	{
		mailbox_data = data;
		OS_Signal(&mailbox_sem);
	}
	EndCritical(PRIMASK_Status);
}

// ******** OS_MailBox_Recv ************
// retreive mail from the MailBox
// Use semaphore to synchronize with OS_MailBox_Send
// Lab 2 spin on semaphore if mailbox empty
// Lab 3 block on semaphore if mailbox empty
// Inputs:  none
// Outputs: data retreived
// Errors:  none
uint32_t OS_MailBox_Recv(void){ uint32_t data;
  //***YOU IMPLEMENT THIS FUNCTION*****
	OS_Wait(&mailbox_sem);
	data = mailbox_data;
  return data;
}


