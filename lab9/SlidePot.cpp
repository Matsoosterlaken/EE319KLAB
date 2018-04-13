// SlidePot.cpp
// Runs on LM4F120/TM4C123
// Provide functions that initialize ADC0
// Last Modified: 4/11/2018 
// Student names: change this to your names or look very silly
// Last modification date: change this to the last modification date or look very silly

#include <stdint.h>
#include "SlidePot.h"
#include "../inc/tm4c123gh6pm.h"

// ADC initialization function 
// Input: none
// Output: none
// measures from PD2, analog channel 5
void ADC_Init(void){ 
	volatile uint32_t delay;
																//PORTD2 Init
	SYSCTL_RCGCGPIO_R |= 0x08;		//Enable clock for Port D
	while((SYSCTL_RCGCGPIO_R & 0x08) == 0){}
	GPIO_PORTD_DIR_R &= ~0x04;
	GPIO_PORTD_AFSEL_R |= 0x04;
	GPIO_PORTD_DEN_R &= ~0x04;
	GPIO_PORTD_AMSEL_R |= 0x04;
																//ADC Init	
	SYSCTL_RCGCADC_R |= 0x01;			//Enables ADC clock
	delay = SYSCTL_RCGCADC_R; // extra time to stabilize
  delay = SYSCTL_RCGCADC_R;       
  delay = SYSCTL_RCGCADC_R;      
  delay = SYSCTL_RCGCADC_R;
	ADC0_PC_R = 0x01;							//125kHz ADC conversion speed
	ADC0_SSPRI_R = 0x0123;				//sequence priority: 3 
	ADC0_ACTSS_R &= ~0x0008;			//Disable selected sequence 3
	ADC0_EMUX_R &= ~0xF000;				//set software start trigger event
	ADC0_SSMUX3_R = (ADC0_SSMUX3_R&0xFFFFFFF0) + 5;				//write channel number in bits 3-0 (input source)
	ADC0_SSCTL3_R = 0x0006;				//set sample control bits to disable temp measurement, notify on sample complete, 
																//indicate single sample in sequence, and denote single-ended signal mode
	ADC0_IM_R &= ~0x0008;					//Disables interrupts
	ADC0_ACTSS_R |= 0x0008;				//Enable selected sequencer 3
}

//------------ADCIn------------
// Busy-wait Analog to digital conversion
// Input: none
// Output: 12-bit result of ADC conversion
// measures from PD2, analog channel 5
uint32_t ADC_In(void){  
  uint32_t data;
  ADC0_PSSI_R = 0x0008;       			//set sofware trigger     
  while((ADC0_RIS_R&0x08)==0){};    //busy-wait: poll until sample complete
  data = ADC0_SSFIFO3_R & 0xFFF; 		//read sample
  ADC0_ISC_R = 0x0008; 							//clear sample complete flag
  return data;
}

// constructor, invoked on creation of class
// m and b are linear calibration coeffients 
// initialize all private variables
// make slope equal to m and offset equal to b
SlidePot::SlidePot(uint32_t m, uint32_t b){
	this->distance = 0;
	this->flag = 0;
	this->data = 0;
	this->slope = m;
	this->offset = b;
}

void SlidePot::Save(uint32_t n){
// 1) save ADC sample into private variable
// 2) calculate distance from ADC, save into private variable
// 3) set semaphore flag = 1
	this->data = n;
	this->distance = this->Convert(n);
	this->flag = 1;
}
uint32_t SlidePot::Convert(uint32_t n){
  // use calibration data to convert ADC sample to distance
	this->slope = 401;
	this->offset = 141070;
  return (this->slope*n + this->offset)/1000;
}


void SlidePot::Sync(void){
// 1) wait for semaphore flag to be nonzero
// 2) set semaphore flag to 0
		while(this->flag == 0){}
		this->flag = 0;
}
uint32_t SlidePot::ADCsample(void){ // return ADC sample value (0 to 4095)
  // return last calculated ADC sample
  return this->data;
}

uint32_t SlidePot::Distance(void){  // return distance value (0 to 2000), 0.001cm
  // return last calculated distance in 0.001cm
  return this->distance;
}


