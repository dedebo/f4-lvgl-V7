#ifndef __LED_H
#define __LED_H
#include "sys.h"


//LED
#define LED0 PFout(9)	//led0
#define LED1 PFout(10)	//led1

void LED_Init(void);//led初始化 				    
#endif
