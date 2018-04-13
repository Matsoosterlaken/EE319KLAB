// main.c
// Runs on LM4F120 or TM4C123
// Student names: put your names here
// Last modification date: change this to the last modification date or look very silly
// Last Modified: 4/11/2018 

// Analog Input connected to PD2=ADC5
// displays on Sitronox ST7735
// PF3, PF2, PF1 are heartbeats
// EE319K Lab 9, use U1Rx connected to PC4 interrupt
// EE319K Lab 9, use U1Tx connected to PC5 busy wait
// EE319K Lab 9 hardware
// System 1        System 2
//   PC4 ----<<----- PC5
//   PC5 ---->>----- PC4
//   Gnd ----------- Gnd

// main1 Understand UART interrupts
// main2 Implement and test the FIFO class on the receiver end 
//   import ST7735 code from Lab 7,8
// main3 convert UART0 to UART1, implement busy-wait on transmission
// final main for Lab 9
//   Import SlidePot and ADC code from Lab8. 
//   Figure out what to do in UART1_Handler ISR (receive message)
//   Figure out what to do in SysTickHandler (sample, convert, transmit message)
//   Figure out what to do in main (LCD output)

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "PLL.h"
#include "ST7735.h"
#include "PLL.h"
#include "SlidePot.h"
#include "print.h"
#include "UART.h"
#include "FIFO.h"

SlidePot my(1500,0);
int TxCounter = 0;
 													

extern "C" void DisableInterrupts(void);
extern "C" void EnableInterrupts(void);
extern "C" void SysTick_Handler(void);

// PF1 should be toggled in UART ISR (receive data)
// PF2 should be toggled in SysTick ISR (60 Hz sampling)
// PF3 should be toggled in main program
#define PF1  (*((volatile uint32_t *)0x40025008))
#define PF2  (*((volatile uint32_t *)0x40025010))
#define PF3  (*((volatile uint32_t *)0x40025020))
#define PF4  (*((volatile uint32_t *)0x40025040))

// **************SysTick_Init*********************
// Initialize Systick periodic interrupts
// Input: interrupt period
//        Units of period are 12.5ns
//        Maximum is 2^24-1
//        Minimum is determined by length of ISR
// Output: none
void SysTick_Init(unsigned long period){
  NVIC_ST_CTRL_R = 0;																						 // disable systick during setup
	NVIC_ST_RELOAD_R = period - 1;																 // maximum reload value 
	NVIC_ST_CURRENT_R = 0;																				 // any write to current clears it
	NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R & 0x00FFFFFF) | 0x40000000; // priority 2
	NVIC_ST_CTRL_R = 0x00000007; 			
}

// Initialize Port F so PF1, PF2 and PF3 are heartbeats
void PortF_Init(void){
  SYSCTL_RCGCGPIO_R |= 0x20;      // activate port F
  while((SYSCTL_PRGPIO_R&0x20) != 0x20){};
  GPIO_PORTF_DIR_R |=  0x0E;   // output on PF3,2,1 (built-in LED)
  GPIO_PORTF_PUR_R |= 0x10;
  GPIO_PORTF_DEN_R |=  0x1E;   // enable digital I/O on PF
}


int main(void){   
  DisableInterrupts(); 
	PLL_Init(Bus80MHz);       // Bus clock is 80 MHz 
  ST7735_InitR(INITR_REDTAB);
  ADC_Init();    // initialize to sample ADC
  PortF_Init();
	SysTick_Init(1333600); // 60Hz
  UART_Init();    // initialize UART 
	char string[8];
  EnableInterrupts(); 

  while(1){ 
//		UART_InMessage(string);
//		ST7735_OutString(string);
//		PF3 ^= 0x08;
  }
}



void SysTick_Handler(void){ // every 16.67 ms
//Similar to Lab8 except rather than grab sample,
// form a message, transmit 
  PF2 ^= 0x04;  											// Heartbeat
	my.Save(ADC_In());
  PF2 ^= 0x04;  											// Heartbeat
  uint8_t data = my.Distance(); 
	char message[8]; 
	
	message[0] = 0x02; 									// STX
  message[1] = (data/1000) + 0x30;    // digit 1
	message[2] = 0x2E;
	message[3] = ((data/100) % 10) + 0x30; // decimal 1
	message[4] = ((data/10) % 10) + 0x30;  // decimal 2
	message[5] = (data % 10) + 0x30;			 // decimal 3
	message[6] = 0x0D; 									// '\r'
	message[7] = 0x03;                  // ETX
	
	for (int i = 0; i < 8; i++) {       // writes to UART_DR_R 8 times 
		UART_OutChar(message[i]); 
	}

	TxCounter++; 
  PF2 ^= 0x04;  											// Heartbeat
}

