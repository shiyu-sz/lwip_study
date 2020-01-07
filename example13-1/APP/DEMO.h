//#include "BUTTON.h"
#ifndef _DEMO_H_ 
#define _DEMO_H_

#define TP_CS()  GPIO_ResetBits(GPIOB,GPIO_Pin_7)	  
#define TP_DCS() GPIO_SetBits(GPIOB,GPIO_Pin_7)	 

#define LED1_ON()	GPIO_SetBits(GPIOD,  GPIO_Pin_2);
#define LED2_ON()	GPIO_SetBits(GPIOA,  GPIO_Pin_8);

#define LED1_OFF()	GPIO_ResetBits(GPIOD,  GPIO_Pin_2);
#define LED2_OFF()	GPIO_ResetBits(GPIOA,  GPIO_Pin_8);
#endif
