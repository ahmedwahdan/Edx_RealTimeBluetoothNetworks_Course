;/*****************************************************************************/
; OSasm.s: low-level OS commands, written in assembly                       */
; Runs on LM4F120/TM4C123/MSP432
; Lab 2 starter file
; February 10, 2016
;


        AREA |.text|, CODE, READONLY, ALIGN=2
        THUMB
        REQUIRE8
        PRESERVE8

        EXTERN  RunPt            ; currently running thread
        EXPORT  StartOS
        EXPORT  SysTick_Handler
        IMPORT  Scheduler

SysTick_Handler   ;; linked with OS_Scheduler function(schedulre written in C)
	CPSID	I
	LDR		R0,=0x400253FC
	LDR		R1,[R0]
	EOR		R1,R1,#0x02
	STR		R1,[R0]
	
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

SysTick_Handlerx                ; 1) Saves R0-R3,R12,LR,PC,PSR
    CPSID   I                  ; 2) Prevent interrupt during switch
	PUSH	{R4-R11}
	
	LDR		R0,=RunPt
	LDR		R1,[R0]
	STR		SP,[R1]
	
	ADD		R1,#4
	LDR		R2,[R1]
	STR		R2,[R0]
	LDR		SP,[R2]
	POP		{R4-R11}
    CPSIE   I                  ; 9) tasks run with interrupts enabled
    BX      LR                 ; 10) restore R0-R3,R12,LR,PC,PSR

StartOS
	CPSID   I     
	LDR		R0,=RunPt
	LDR		R1,[R0]
	LDR		SP,[R1]
	POP		{R4-R11}
	POP		{R0-R3}
	POP		{R12}
	ADD		SP,#4
	POP		{LR}
	ADD		SP,#4
    CPSIE   I                  ; Enable interrupts at processor level
    BX      LR                 ; start first thread

    ALIGN
    END
