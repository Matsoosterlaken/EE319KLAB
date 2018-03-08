// TableTrafficLight.c edX lab 10, EE319K Lab 5
// Runs on LM4F120 or TM4C123
// Index implementation of a Moore finite state machine to operate a traffic light.  
// Ryan Taylor, Mats Oosterlaken
// Mar 7, 2018

/* 
 Copyright 2018 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

// See TExaS.h for other possible hardware connections that can be simulated
// east/west red light connected to PE5
// east/west yellow light connected to PE4
// east/west green light connected to PE3
// north/south facing red light connected to PE2
// north/south facing yellow light connected to PE1
// north/south facing green light connected to PE0
// pedestrian detector connected to PA4 (1=pedestrian present)
// north/south car detector connected to PA3 (1=car present)
// east/west car detector connected to PA2 (1=car present)
// "walk" light connected to PF3,2,1 (built-in white LED)
// "don't walk" light connected to PF1 (built-in red LED)
#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "SysTick.h"
#include "TExaS.h"
#define SouthGreen 0
#define SouthYellow 1
#define SouthRed 2
#define SouthYellowB 3
#define SouthRedB 4
#define WestGreen 5
#define WestYellow 6
#define WestRed 7
#define WestGreenB 18
#define WestYellowB 8
#define WestRedB 9
#define WalkWhite 10
#define WalkRed1 11
#define WalkOff1 12
#define WalkRed2 13
#define WalkOff2 14
#define WalkRed3 15
#define WalkOff3 16
#define WalkRed4 17

 typedef const struct State {
		uint32_t EOut;
		uint32_t FOut;
		uint32_t Delay;
		uint32_t Next[8];
	}state_type;
	
	uint32_t current_state = 10; 		
	uint8_t input = 0;
	uint32_t i;

// initializing Array of States
	state_type FSM[19] = {
			{0x21,0x02,200,{SouthGreen,SouthYellow,SouthGreen,SouthYellow,SouthYellowB,SouthYellowB,SouthYellowB,SouthYellowB}}, 
			{0x22,0x02,100,{SouthRed,SouthRed,SouthRed,SouthRed,SouthRedB,SouthRedB,SouthRedB,SouthRedB}},
			{0x24,0x02,100,{WestGreen,WestGreen,WestGreen,WestGreen,WalkWhite,WalkWhite,WalkWhite,WalkWhite}}, 
			{0x22,0x02,100,{SouthRedB,SouthRedB,SouthRedB,SouthRedB,SouthRedB,SouthRedB,SouthRedB,SouthRedB}}, 
			{0x24,0x02,100,{WalkWhite,WestGreenB,WalkWhite,WestGreenB,WalkWhite,WestGreenB,WalkWhite,WestGreenB}}, 
			{0x0C,0x02,200,{WestGreen,WestGreen,WestYellow,WestYellow,WestYellowB,WestYellowB,WestYellowB,WestYellowB}}, 
			{0x14,0x02,100,{WestRed,WestRed,WestRed,WestRed,WestRedB,WestRedB,WestRedB,WestRedB}}, 
			{0x24,0x02,100,{SouthGreen,SouthGreen,SouthGreen,SouthGreen,WalkWhite,WalkWhite,WalkWhite,WalkWhite}}, 
			{0x14,0x02,100,{WestRedB,WestRedB,WestRedB,WestRedB,WestRedB,WestRedB,WestRedB,WestRedB}}, 
			{0x24,0x02,100,{WalkWhite,WalkWhite,WalkWhite,WalkWhite,WalkWhite,WalkWhite,WalkWhite,WalkWhite}}, 
			{0x24,0x0E,,{WalkWhite,WalkRed1,WalkRed1,WalkRed1,WalkWhite,WalkRed1,WalkRed1,WalkRed1}}, 
			{0x24,0x02,20,{WalkOff1,WalkOff1,WalkOff1,WalkOff1,WalkOff1,WalkOff1,WalkOff1,WalkOff1}}, 							// flashing walk delay is shorter
			{0x24,0x00,20,{WalkRed2,WalkRed2,WalkRed2,WalkRed2,WalkRed2,WalkRed2,WalkRed2,WalkRed2}}, 
			{0x24,0x02,20,{WalkOff2,WalkOff2,WalkOff2,WalkOff2,WalkOff2,WalkOff2,WalkOff2,WalkOff2}}, 
			{0x24,0x00,20,{WalkRed3,WalkRed3,WalkRed3,WalkRed3,WalkRed3,WalkRed3,WalkRed3,WalkRed3}}, 
			{0x24,0x02,20,{WalkOff3,WalkOff3,WalkOff3,WalkOff3,WalkOff3,WalkOff3,WalkOff3,WalkOff3}}, 
			{0x24,0x00,20,{WalkRed4,WalkRed4,WalkRed4,WalkRed4,WalkRed4,WalkRed4,WalkRed4,WalkRed4}}, 
			{0x24,0x02,100,{WestGreen,WestGreen,SouthGreen,SouthGreen,WestGreen,WestGreen,SouthGreen,SouthGreen}}, 
			{0x0C,0x02,200,{WestYellowB,WestYellowB,WestYellowB,WestYellowB,WestYellowB,WestYellowB,WestYellowB,WestYellowB}} };			

void PortF_Init(void){ 																							// PF3-1 are outputs
	volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x00000020;     																// 1) activate clock for Port F
  delay = SYSCTL_RCGC2_R;          																	// allow time for clock to start
  GPIO_PORTF_LOCK_R = 0x4C4F434B;   																// 2) unlock GPIO Port F
  GPIO_PORTF_CR_R = 0x1F;           																// allow changes to PF4-0
																																		// only PF0 needs to be unlocked, other bits can't be locked
  GPIO_PORTF_AMSEL_R = 0x00;        																// 3) disable analog on PF
	GPIO_PORTF_PCTL_R = 0x00000000;  						 											// 4) PCTL GPIO on PF3-0
  GPIO_PORTF_DIR_R = ((GPIO_PORTF_DIR_R & ~(0x0E)) | 0x0E);         // 5) PF3-1 out
  GPIO_PORTF_AFSEL_R = 0x00;        																// 6) disable alt funct on PF3-1
  GPIO_PORTF_DEN_R = 0x0E;          																// 7) enable digital I/O on PF3-1
}
void PortE_Init(){																									// PE0-5 are outputs
	volatile unsigned long delay;
	SYSCTL_RCGC2_R |= 0x00000010;     																// 1) activate clock for Port E
  delay = SYSCTL_RCGC2_R;          													 				// allow time for clock to start
  GPIO_PORTE_AMSEL_R = 0x00;        																// 3) disable analog on PE
  GPIO_PORTE_PCTL_R = 0x00000000;   																// 4) PCTL GPIO on PE5-0
  GPIO_PORTE_DIR_R = ((GPIO_PORTE_DIR_R & ~(0x3F)) | 0x3F);         // 5) PE5-0 out
  GPIO_PORTE_AFSEL_R = 0x00;       													 				// 6) disable alt funct on PE5-0
  GPIO_PORTE_DEN_R = 0x3F;          																// 7) enable digital I/O on PE5-0
}

void PortA_Init(){																									// PA4-2 are inputs 
	volatile unsigned long delay;
	SYSCTL_RCGC2_R |= 0x0000001;      																// 1) activate clock for Port A
  delay = SYSCTL_RCGC2_R;           																// allow time for clock to start
  GPIO_PORTA_AMSEL_R = 0x00;        																// 3) disable analog on PA
  GPIO_PORTA_PCTL_R = 0x00000000;   																// 4) PCTL GPIO on PA4-2
  GPIO_PORTA_DIR_R = (GPIO_PORTF_DIR_R & ~(0x1C));        					// 5) PA4-2 in
  GPIO_PORTA_AFSEL_R = 0x00;        																// 6) disable alt funct on PA4-2
  GPIO_PORTA_DEN_R = 0x1C;         	 																// 7) enable digital I/O on PE4-2
}

void EnableInterrupts(void);


// output Led configuration 
void TrafficLights_Out(uint8_t stoplights, uint8_t walksign){			
	GPIO_PORTE_DATA_R = ((GPIO_PORTE_DATA_R & ~(0x3F)) | stoplights ); // PE5-0 are set 
	GPIO_PORTF_DATA_R = ((GPIO_PORTF_DATA_R & ~(0x0E)) | walksign ); 	 // PF3-1 are set 
}

// get input configuration to go to next state
uint8_t getInputs(){
	uint8_t out = 0;
	out = GPIO_PORTA_DATA_R;
	out &= 0x1C;
	out = out>>2;
	return out;
}	


void EnableInterrupts(void);

int main(void){ 
  TExaS_Init(SW_PIN_PA432, LED_PIN_PE543210); 												// activate traffic simulation and set system clock to 80 MHz
  SysTick_Init();     
  PortF_Init();																												// PF3-1 are outputs
	PortE_Init(); 																											// PE5-0 are outputs
	PortA_Init();																												// PA4-2 are inputs 
  EnableInterrupts(); 																								// TExaS uses interrupts
	
  while(1){
		TrafficLights_Out(FSM[current_state].EOut, FSM[current_state].FOut); 
		SysTick_Wait10ms(FSM[current_state].Delay); 											
		input = getInputs();																				
		current_state = FSM[current_state].Next[input];    
  }
}

