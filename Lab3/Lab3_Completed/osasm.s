;/*****************************************************************************/
; OSasm.s: low-level OS commands, written in assembly                       */
; Runs on LM4F120/TM4C123/MSP432
; Lab 3 starter file
; March 2, 2016




        AREA |.text|, CODE, READONLY, ALIGN=2
        THUMB
        REQUIRE8
        PRESERVE8

        EXTERN  RunPt            ; currently running thread
        EXPORT  StartOS
        EXPORT  SysTick_Handler
        IMPORT  Scheduler


SysTick_Handler                ; 1) Saves R0-R3,R12,LR,PC,PSR
    CPSID	I
	;LDR		R0,=0x400253FC
	;LDR		R1,[R0]
	;EOR		R1,R1,#0x02
	;STR		R1,[R0]
	
	PUSH	{R4-R11}				; PUSH remaining registers
	LDR		R0,=RunPt	; Load Run_Task_TCB_Ptr address
	LDR		R1,[R0]					; Load its value (The current running task TCB address)
	STR		SP,[R1]					; Store the SP value into the TCB SP value	
	
	PUSH	{R0,LR}
	BL		Scheduler
	POP		{R0,LR}
	
	;;LDR		R0,=Run_Task_TCB_Ptr	; Load Run_Task_TCB_Ptr address
	LDR		R1,[R0]					; Load its value (The current running task TCB address)
	LDR		SP,[R1]					; Update the SP with the current task SP
	POP		{R4-R11}				; POP R4 to R11	
	CPSIE	I
	BX		LR

StartOS
   ;YOU IMPLEMENT THIS (same as Lab 2)

    LDR	R0,=RunPt   	;address of Run_Task_TCB_Ptr
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

    ALIGN
    END
