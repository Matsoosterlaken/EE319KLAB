;****************** main.s ***************
; Program written by: put your names here Ryan Taylor
; Date Created: 8/25/2013 
; Last Modified: 3/18/2018 
; Section 1-2pm     TA: Youngchun Kim
;****************** main.s ***************
; Program written by: Matteus Oosterlaken & Ryan Taylor
; Date Created: 8/25/2013 
; Last Modified: 2/24/2015 
; Section 1-2pm     TA: Youngchun Kim
; Lab number: 5
; Brief description of the program
;   A traffic light controller with 3 inputs and 8 output
; Hardware connections
;The “don’t walk” and “walk” lights must be PF1 and PF3 respectively, but where to attach the others have some flexibility. 
;Obviously, you will not connect both inputs and outputs to the same pin.

;Red south       PA7    PB5    PE5
;Yellow south    PA6    PB4    PE4
;Green south     PA5    PB3    PE3
;Red west        PA4    PB2    PE2
;Yellow west     PA3    PB1    PE1
;Green west      PA2    PB0    PE0
;Table 5.1. Possible ports to interface the traffic lights (PF1=red don’t walk, PF3=green walk).

;Walk sensor     PA4    PD2    PE2
;South sensor    PA3    PD1    PE1
;West sensor     PA2    PD0    PE0
;Table 5.2. Possible ports to interface the sensors.

SYSCTL_RCGCGPIO_R       EQU 0x400FE608
GPIO_PORTA_DATA_R       EQU 0x400043FC
GPIO_PORTA_DIR_R        EQU 0x40004400
GPIO_PORTA_AFSEL_R      EQU 0x40004420
GPIO_PORTA_DEN_R        EQU 0x4000451C
GPIO_PORTB_DATA_R       EQU 0x400053FC
GPIO_PORTB_DIR_R        EQU 0x40005400
GPIO_PORTB_AFSEL_R      EQU 0x40005420
GPIO_PORTB_DEN_R        EQU 0x4000551C
GPIO_PORTD_DATA_R       EQU 0x400073FC
GPIO_PORTD_DIR_R        EQU 0x40007400
GPIO_PORTD_AFSEL_R      EQU 0x40007420
GPIO_PORTD_DEN_R        EQU 0x4000751C
GPIO_PORTE_DATA_R       EQU 0x400243FC
GPIO_PORTE_DIR_R        EQU 0x40024400
GPIO_PORTE_AFSEL_R      EQU 0x40024420
GPIO_PORTE_DEN_R        EQU 0x4002451C
GPIO_PORTF_DATA_R       EQU 0x400253FC
GPIO_PORTF_DIR_R        EQU 0x40025400
GPIO_PORTF_AFSEL_R      EQU 0x40025420
GPIO_PORTF_DEN_R        EQU 0x4002551C
NVIC_ST_CURRENT_R       EQU 0xE000E018
NVIC_ST_CTRL_R          EQU 0xE000E010
NVIC_ST_RELOAD_R        EQU 0xE000E014
KEY						EQU 0x4C4F434B
DELAY					EQU 799999



      AREA   DATA, ALIGN=2
current_state SPACE 4	
state_ptr 	SPACE 4 	
	


      ALIGN     		  
      AREA    |.text|, CODE, READONLY, ALIGN=2	 
      THUMB
      EXPORT  Start
      IMPORT  PLL_Init
	

Start

state0 DCD	0x21, 0x02, 100, 0, 1, 0, 1, 3, 3, 3, 3  	
state1 DCD	0x22, 0x02, 100, 2, 2, 2, 2, 4, 4, 4, 4
state2 DCD  0x24, 0x02, 100, 5, 5, 5, 5, 10, 10, 10, 10 
state3 DCD  0x22, 0x02, 100, 4, 4, 4, 4, 4, 4, 4, 4	
state4 DCD  0x24, 0x02, 100, 10, 18, 10, 18, 10, 18, 10, 18	
state5 DCD  0x0C, 0x02, 100, 5, 5, 6, 6, 8, 8, 8, 8	
state6 DCD  0x14, 0x02, 100, 7, 7, 7, 7, 9, 9, 9, 9	
state7 DCD  0x24, 0x02, 100, 0, 0, 0, 0, 10, 10, 10, 10	
state8 DCD  0x14, 0x02, 100, 9, 9, 9, 9, 9, 9, 9, 9	
state9 DCD  0x24, 0x02, 100, 10, 10, 10, 10, 10, 10, 10, 10	
state10 DCD 0x24, 0x08, 100, 10, 11, 11, 11, 10, 11, 11, 11	
state11 DCD 0x24, 0x02, 100, 12, 12, 12, 12, 12, 12, 12, 12	
state12 DCD 0x24, 0x00, 100, 13, 13, 13, 13, 13, 13, 13, 13	
state13 DCD 0x24, 0x02, 100, 14, 14, 14, 14, 14, 14, 14, 14	
state14 DCD 0x24, 0x00, 100, 15, 15, 15, 15, 15, 15, 15, 15	
state15 DCD	0x24, 0x02, 100, 16, 16, 16, 16, 16, 16, 16, 16
state16 DCD 0x24, 0x00, 100, 17, 17, 17, 17, 17, 17, 17, 17 	
state17 DCD 0x24, 0x02, 100, 5, 5, 0, 0, 5, 5, 0, 0 	
state18 DCD 0x0C, 0x02, 100, 8, 8, 8, 8, 8, 8, 8, 8 	

; the DCM.DLL -pCM4 -dLaunchPadDLL debugger setting does NOT grade or simulate traffic
; the DCM.DLL -pCM4 -dEE319KLab5 debugger setting DOES not test all specifications for EE319K
     BL   PLL_Init    ; running at 80 MHz
	 BL	  SysTick_Init
	 BL	  PortF_Init
	 BL   PortE_Init
	 BL   PortA_Init
	 BL   EnableInterrupts
	 mov R0, #0x21
	 mov r1, #0x02
	 BL	  TrafficLights_Out
	 
	 LDR R0,= current_state
	 MOV R1, #0					; setting intial state to state 0 
	 STR R1, [R0]
	 
	 LDR R0,= state_ptr
	 LDR R1,= state0			; state_ptr points to first state
	 STR R1, [R0] 			
	 
	 MOV R4, #4 				; R4 = 4
	 MOV R11, #11				; R11 = 11
	 

loop  
	 MOV R7, #0 
	 LDR R2,= state_ptr
	 LDR R2, [R2]					; R2 = Location of first state 
	 LDR R3,= current_state
	 LDR R3, [R3]					; R3 = current state
	 MUL R3, R3, R4				 
	 MUL R3, R3, R11 				; R3(11*4) = index between states
	 ADD R7, R2, R3					; R7 = location of E.out (location of first state + current_state(ll*4) 
	 LDR R0, [R7]					; R0 = E.out
	 ADD R7, #4						; R7 = location of F.out
	 LDR R1, [R7]					; R1 = F.out
	 ;R0 <- FSM[current_state].EOut
	 ;R1 <- FSM[current_state].FOut
	 BL	  TrafficLights_Out
	 ADD R7, #4						; R7 = location of Delay
	 LDR R0, [R7]
	 ;R0 <- FSM[current_state].Delay
	 BL   SysTick_Wait10ms
	 BL   getInputs
	 ;R0 contains input from getInputs 
	 ADD R7, R7, #4 
	 MUL R0, R4						; input * offset for 32 bit array 
	 ADD R7, R7, R0					; R7 = location of Next State
	 LDR R2, [R7] 					; R2 = next state number
	 LDR R3,= current_state
	 STR R2, [R3] 					; updating current state	
	 ;current_state = FSM[current_state].Next[input];
	  B    loop

SysTick_Init
	LDR R0,= NVIC_ST_CTRL_R
	MOV R1, #0
	STR R1, [R0]
	LDR R0,= NVIC_ST_RELOAD_R
	MOV R1, #0x00FFFFFF
	STR R1, [R0]
	LDR R0,= NVIC_ST_CURRENT_R
	MOV R1, #0
	STR R1, [R0]
	LDR R0,= NVIC_ST_CTRL_R
	MOV R1, #0x00000005
	STR R1, [R0]
	BX LR

PortF_Init
	LDR R0,= SYSCTL_RCGCGPIO_R
	LDR R1, [R0]
	ORR R1, #0x00000020
	STR R1, [R0]
	NOP
	NOP
	LDR R0,= GPIO_PORTF_DIR_R
	LDR R1, [R0]
	MOV R2, #0x0E
	ORR R1,R2
	STR R1, [R0]
	LDR R0, =GPIO_PORTF_AFSEL_R
	MOV R1, #0x00
	STR R1, [R0]
	LDR R0, =GPIO_PORTF_DEN_R
	MOV R1, #0x0E
	STR R1, [R0]
	BX LR
	
PortE_Init
	LDR R0,= SYSCTL_RCGCGPIO_R
	LDR R1, [R0]
	ORR R1, #0x00000010
	STR R1, [R0]
	NOP
	NOP
;	LDR R0,= GPIO_PORTE_AMSEL_R
;	MOV R1, #0x00
;	STR R1, [R0]
;	LDR R0,= GPIO_PORTE_PCTL_R
;	MOV R1, #0x00000000
;	STR R1, [R0]
	LDR R0,= GPIO_PORTE_DIR_R
	LDR R1, [R0]
	ORR R1, #0x3F
	STR R1, [R0]
	LDR R0, =GPIO_PORTE_AFSEL_R
	MOV R1, #0x00
	STR R1, [R0]
	LDR R0, =GPIO_PORTE_DEN_R
	MOV R1, #0x3F
	STR R1, [R0]
	BX LR
	
PortA_Init
	LDR R0,= SYSCTL_RCGCGPIO_R
	LDR R1, [R0]
	ORR R1, #0x0000001
	STR R1, [R0]
	NOP
	NOP
;	LDR R0,= GPIO_PORTA_AMSEL_R
;	MOV R1, #0x00
;	STR R1, [R0]
;	LDR R0,= GPIO_PORTA_PCTL_R
;	MOV R1, #0x00000000
;	STR R1, [R0]
	LDR R0,= GPIO_PORTA_DIR_R
	LDR R1, [R0]
	MOV R2, #0x1C
	BIC R1,R2
	STR R1, [R0]
	LDR R0, =GPIO_PORTA_AFSEL_R
	MOV R1, #0x00
	STR R1, [R0]
	LDR R0, =GPIO_PORTA_DEN_R
	MOV R1, #0x1C
	STR R1, [R0]
	BX LR
	
EnableInterrupts
	BX LR
	
TrafficLights_Out
	LDR R2,= GPIO_PORTE_DATA_R
	LDR R3, [R2]
	BIC r3, #0x3F
	ORR R3, R0
	STR R3, [R2]
	LDR R2,= GPIO_PORTF_DATA_R
	LDR R3, [R2]
	BIC r3, #0x0E
	ORR R3, R1
	STR R3, [R2] 
	BX LR

SysTick_Wait10ms	;R0 Contains Delay
	MOV R1, #0
Wait_Loop	
	LDR R2,= NVIC_ST_RELOAD_R
	LDR R3,= DELAY
	SUB R3, #1
	STR R3, [R2]
	LDR R2,= NVIC_ST_CURRENT_R
	MOV R3, #0
	STR R3, [R2]
Wait
	LDR R2,= NVIC_ST_CTRL_R
	LDR R3, [R2]
	CMP R3, #0x00010000
	BEQ Wait
	ADDS R1, #1
	CMP R1,R0
	BEQ done
	BNE Wait_Loop
done	BX LR
	
getInputs ; R0 gets inputs b00000XXX
	LDR R1,= GPIO_PORTA_DATA_R
	LDR R2, [R1]
	AND R2, #0x1C
	LSR R2, #2
	MOV R0, R2
	BX LR
		
	
      ALIGN          ; make sure the end of this section is aligned
      END            ; end of file
      