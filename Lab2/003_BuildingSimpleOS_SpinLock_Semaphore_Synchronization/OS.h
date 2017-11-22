#ifndef OS_H
#define OS_H


/********************************************************/
/*										definitions												*/
/********************************************************/
#define CPU_CLOCK_HZ					(50000000UL)
#define SYS_TICKS_PER_SEC			(1000U)

/********************************************************/
/*										Prototypes												*/
/********************************************************/
void OS_Start(void);
void _Disable_Global_Interrupt(void);
void _Enable_Global_Interrupt(void);
#endif /*	OS_H	*/
