		AREA |.text|, CODE, READONLY, ALIGN=2
        THUMB
        REQUIRE8
        PRESERVE8
		
		EXTERN 	Run_Task_TCB_Ptr	
		EXPORT	_OS_Start
		EXPORT	SysTick_Handler
		
		IMPORT	OS_Scheduler	


_OS_Start
		LDR	R0,=Run_Task_TCB_Ptr   	;address of Run_Task_TCB_Ptr
		LDR	R1,[R0]					;value of Run_Task_TCB_Ptr pointer
		LDR	SP,[R1]					;value of SP of the TCB that Run_Task_TCB_Ptr points to
		POP	{R4-R11}				;POP R4 to R11 from task stack to CPU registers
		POP	{R0-R3}
		POP	{R12}
		ADD	SP,SP,#4
		POP	{LR}
		ADD	SP,SP,#4
		CPSIE   I   
		BX	LR
		
			


SysTick_Handler   ;; linked with OS_Scheduler function(schedulre written in C)
	CPSID	I
	LDR		R0,=0x400253FC
	LDR		R1,[R0]
	EOR		R1,R1,#0x02
	STR		R1,[R0]
	
	PUSH	{R4-R11}				; PUSH remaining registers
	LDR		R0,=Run_Task_TCB_Ptr	; Load Run_Task_TCB_Ptr address
	LDR		R1,[R0]					; Load its value (The current running task TCB address)
	STR		SP,[R1]					; Store the SP value into the TCB SP value	
	
	PUSH	{R0,LR}
	BL		OS_Scheduler
	POP		{R0,LR}
	
	;;LDR		R0,=Run_Task_TCB_Ptr	; Load Run_Task_TCB_Ptr address
	LDR		R1,[R0]					; Load its value (The current running task TCB address)
	LDR		SP,[R1]					; Update the SP with the current task SP
	POP		{R4-R11}				; POP R4 to R11	
	CPSIE	I
	BX		LR

	
SysTick_Handlerx   ;; Direct switching
	CPSID	I
	LDR		R0,=0x400253FC
	LDR		R1,[R0]
	EOR		R1,R1,#0x02
	STR		R1,[R0]
	
	PUSH	{R4-R11}				; PUSH remaining registers
	LDR		R0,=Run_Task_TCB_Ptr	; Load Run_Task_TCB_Ptr address
	LDR		R1,[R0]					; Load its value (The current running task TCB address)
	STR		SP,[R1]					; Store the SP value into the TCB SP value	
	ADD		R1,#4				    ; Add 4 to the R1(the running task TCB) and this will point to the 'next_TCB' variable
	LDR		R2,[R1]					; Load the next TCB address into R2
	STR		R2,[R0]					; Update the Run_Task_TCB_Ptr value to point to the next task TCB
	LDR		SP,[R2]					; Update the SP with the current task SP
	POP		{R4-R11}				; POP R4 to R11	
	CPSIE	I
	BX		LR






		ALIGN
		END			