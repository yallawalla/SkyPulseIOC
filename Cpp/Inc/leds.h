#ifndef LEDS_H
#define LEDS_H
#include "stm32f4xx_hal.h"
#include "proc.h"

class _LED {
	private:
		static uint32_t				timeout[];
		static GPIO_TypeDef*	gpio[]; 
		static uint32_t				pin[];
	public:
		_LED() {
			_proc_add((void *)poll,NULL,(char *)"leds",10);
		}
		
	 static void poll(void *);
	 
	 void RED1(int32_t t)			{ timeout[0]=t+HAL_GetTick(); };
	 void RED2(int32_t t)			{ timeout[4]=t+HAL_GetTick(); };
	 void GREEN1(int32_t t)		{ timeout[1]=t+HAL_GetTick(); };
	 void GREEN2(int32_t t)		{ timeout[5]=t+HAL_GetTick(); };
	 void YELLOW1(int32_t t)	{ timeout[2]=t+HAL_GetTick(); };
	 void YELLOW2(int32_t t) 	{ timeout[6]=t+HAL_GetTick(); };
	 void BLUE1(int32_t t)		{ timeout[3]=t+HAL_GetTick(); };
	 void BLUE2(int32_t t)		{ timeout[7]=t+HAL_GetTick(); };
};
#endif
