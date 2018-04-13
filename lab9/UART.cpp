// UART.cpp
// Runs on LM4F120/TM4C123
// This code runs on UART0 with interrupts and a simple FIFO
// EE319K tasks
// 1) Convert to UART1 PC4 PC5
// 2) Implement the FIFO as a class
// 3) Run transmitter with busy-wait synchronization
// 4) Run receiver with 1/2 full FIFO interrupt
// Daniel and Jonathan Valvano
// April 11, 2018


/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2017
   Program 5.11 Section 5.6, Program 3.10

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

// U0Rx (VCP receive) connected to PA0
// U0Tx (VCP transmit) connected to PA1
// EE319K Lab 9, use U1Rx connected to PC4 interrupt
// EE319K Lab 9, use U1Tx connected to PC5 busy wait
// EE319K Lab 9 hardware
// System 1        System 2
//   PC4 ----<<----- PC5
//   PC5 ---->>----- PC4
//   Gnd ----------- Gnd
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"

#include "FIFO.h"
#include "UART.h"

#define PF1  (*((volatile uint32_t *)0x40025008))

extern "C" void DisableInterrupts(void); // Disable interrupts
extern "C" long StartCritical (void);    // previous I bit, disable interrupts
extern "C" void EndCritical(long sr);    // restore I bit to previous value
extern "C" void WaitForInterrupt(void);  // low power mode
extern "C" void EnableInterrupts(void);

#define starter 0
// 1 means UART0, TX/RX interrupts, simple FIFO
// 0 means your Lab 9
#if starter
// EE319K - do not use these two FIFOs
// Two-index implementation of the transmit FIFO
// can hold 0 to TXFIFOSIZE elements
#define NVIC_EN0_INT5    0x00000020  // Interrupt 5 enable
extern "C" void UART0_Handler(void);
#define TXFIFOSIZE 32 // must be a power of 2
#define FIFOSUCCESS 1
#define FIFOFAIL    0


// ************Lab 9 TO DO**********

#else
extern "C" void UART1_Handler(void);
#define NVIC_EN0_INT6   0x00000040  		// Interrupt 6 enable
Queue RxFifo; 													// static implementation of class
int count;  														// global error count 
int RxCounter = 0; 											// number of UART messages recieved 

// Initialize UART0
// Baud rate is 115200 bits/sec
// Lab 9
void UART_Init(void){
	count = 0; 														// clear global error count
  SYSCTL_RCGCUART_R |= 0x0002;            // activate UART1
  SYSCTL_RCGCGPIO_R |= 0x0004;            // activate port c
  
	
	RxFifo = Queue(); 
	
  UART1_CTL_R &= ~0x0001;								// disable UART
  UART1_IBRD_R = 43;                    // IBRD = int(80,000,000 / (16 * 115,200)) = int(43.403)
  UART1_FBRD_R = 26;                    // FBRD = round(0.403 * 64 ) = 26
                                        
  UART1_LCRH_R = 0x0070; // 8 bit word length (no parity bits, one stop bit, FIFOs)
//  UART1_IFLS_R &= ~0x3F;                // clear TX and RX interrupt FIFO level fields
//  UART1_IFLS_R |= 0x08;                  // bits 5,4,3 to 010 (1/2 full)
                                        // configure interrupt for RX FIFO >= 1/2 full
                                        // enable TX and RX FIFO interrupts and RX time-out interrupt
//	UART1_IM_R |= 0x10;										// Arm RXRIS  
  UART1_CTL_R |= 0x301;                 // enable UART
	
	
  GPIO_PORTC_AFSEL_R |= 0x30;
  (GPIO_PORTC_PCTL_R &= 0xFF00FFFF) += 0x00110000;
	GPIO_PORTC_DEN_R |= 0x30;
	GPIO_PORTC_AMSEL_R &= ~0x30;
	
                                        // UART0=priority 2
// NVIC_PRI1_R = (NVIC_PRI1_R&0xFF0FFFFF)|0x00E00000; // bits 21-23
//  NVIC_EN0_R = NVIC_EN0_INT6;           // enable interrupt 6 in NVIC
}


// input ASCII character from UART
// spin if RxFifo is empty
// Lab 9
char UART_InChar(void){
	while ((UART1_FR_R & 0x0010) != 1) {}
	return ((char)(UART1_DR_R&0xFF)); 
}
// Lab 9
bool UART_InStatus(void){
  return 0; 
}


void UART_OutChar(char data){
// output ASCII character to UART
// busy-wait spin if hardware not ready
// Lab 9
// in Lab 9 this will never wait
  while ((UART1_FR_R & 0x0020) != 0){} // wait untill TXFF is 0 
	UART1_DR_R = data; 	
}



// one thing has happened:
// hardware RX FIFO goes from 7 to 8 items
// Lab 9
void UART1_Handler(void){
  PF1  ^= 0x02; 														// Heartbeat
  PF1  ^= 0x02;															// Heartbeat
  while ((UART1_FR_R & 0x0010) == 0) {      // run while RXFE is not empty 
		RxFifo.Put(UART_InChar());              // putting all bytes into software FIFO
	}
	
	RxCounter++; 
	UART1_ICR_R = 0x10; 											// acknowledge interrupt, clear RXRIS in the RIS Register
  PF1  ^= 0x02;															// Heartbeat 
}



#endif

//------------UART_OutString------------
// Output String (NULL termination)
// Input: pointer to a NULL-terminated string to be transferred
// Output: none
void UART_OutString(char *pt){
  while(*pt){
    UART_OutChar(*pt);
    pt++;
  }
}



//-----------------------UART_OutUDec-----------------------
// Output a 32-bit number in unsigned decimal format
// Input: 32-bit number to be transferred
// Output: none
// Variable format 1-10 digits with no space before or after
void UART_OutUDec(uint32_t n){
// This function uses recursion to convert decimal number
//   of unspecified length as an ASCII string
  if(n >= 10){
    UART_OutUDec(n/10);
    n = n%10;
  }
  UART_OutChar(n+'0'); /* n is between 0 and 9 */
}

//------------UART_InMessage------------
// Accepts ASCII characters from the serial port
//    and adds them to a string until ETX is typed
//    or until max length of the string is reached.
// Input: pointer to empty buffer of 8 characters
// Output: Null terminated string
void UART_InMessage(char *bufPt){
	char *data;
	while (RxFifo.IsEmpty())
	RxFifo.Get(data);
	int index = 0;
	while (*data != ETX){
		if ((*data != ETX) && (*data != STX) && (*data != '\r')){
			bufPt[index] = *data;
			index++;
		}
		while (RxFifo.IsEmpty())
		RxFifo.Get(data);
	}
	bufPt[index] = '\0';; 
}
