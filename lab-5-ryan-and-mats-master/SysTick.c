// SysTick.c
// Implements two busy-wait based delay routines
#include <stdint.h>
// Initialize SysTick with busy wait running at bus clock.
#define NVIC_ST_CTRL_R      (*((volatile unsigned long *)0xE000E010))
#define NVIC_ST_RELOAD_R    (*((volatile unsigned long *)0xE000E014))
#define NVIC_ST_CURRENT_R   (*((volatile unsigned long *)0xE000E018))
	
void SysTick_Init(void){
	NVIC_ST_CTRL_R = 0;										// disable systick during setup
	NVIC_ST_RELOAD_R = 0x00FFFFFF;				// maximum reload value
	NVIC_ST_CURRENT_R = 0;								// any write to current clears it
	NVIC_ST_CTRL_R = 0x00000005; 					// enable systick with core clock
}

// The delay parameter is in units of the 80 MHz core clock. (12.5 ns)
void SysTick_Wait(uint32_t delay){
  NVIC_ST_RELOAD_R = delay-1;  					 // number of counts
  NVIC_ST_CURRENT_R = 0;      					 // any value written to CURRENT clears
  while((NVIC_ST_CTRL_R&0x00010000)==0){
	}
}

// Time delay using busy wait.
// waits for count*10ms
// 10000us equals 10ms
void SysTick_Wait10ms(uint32_t delay){
	unsigned long i; 
	for(i=0; i<delay; i++){
   SysTick_Wait(800000); 								// wait 10ms
	}
} 

