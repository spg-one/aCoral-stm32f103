#ifndef _LED_H
#define _LED_H
#include "sys.h"

#define LED0 PAout(8)   	//LED0
#define LED1 PDout(2)   	//LED1

void LED_Init(void);
#endif
