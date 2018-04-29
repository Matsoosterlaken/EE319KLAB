

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"

void DAC_Init(void){	// initialize PB5-0 out
	volatile unsigned long delay;
	SYSCTL_RCGCGPIO_R |= 0x00000002;     																// 1) activate clock for Port B
  delay = SYSCTL_RCGCGPIO_R;          													 				// allow time for clock to start
  GPIO_PORTB_AMSEL_R &= ~0x01;        															// 3) disable analog on PB
  GPIO_PORTB_PCTL_R = 0x00000000;   																// 4) PCTL GPIO on PB5-0
  GPIO_PORTB_DIR_R = ((GPIO_PORTB_DIR_R & ~(0x3F)) | 0x3F);         // 5) PB5-0 out
  GPIO_PORTB_AFSEL_R = 0x00;       													 				// 6) disable alt funct on PB5-0
  GPIO_PORTB_DEN_R = 0x3F;          																// 7) enable digital I/O on PB5-0
}


void DAC_Out(uint32_t data){			
	GPIO_PORTB_DATA_R = ((GPIO_PORTB_DATA_R & ~(0x3F)) | data); 
}

