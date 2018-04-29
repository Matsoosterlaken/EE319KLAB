
#ifndef __SOUND_H__ // do not include more than once
#define __SOUND_H__
#include <stdint.h>
#include "DAC.h"

#include <stdint.h>

void Sound_Init(void);
void Sound_Play(uint32_t period);
void Sound_Sword(void); 
void Sound_FireBall(void);
void Sound_Death(void);
void Sound_Damage(void);
void Sound_Chest(void); 
void Sound_Room(void); 
void Sound_Shoot(); 
void Sound_Hit(); 
#endif


