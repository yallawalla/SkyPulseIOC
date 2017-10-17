#ifndef LEDS_H
#define LEDS_H
#include "stm32f4xx_hal.h"
#include "proc.h"
#include "algorithm"

class _LED {
	private:
		uint32_t			timeout[8];
		GPIO_TypeDef*	gpio[8]; 
		uint32_t			pin[8];
	public:
		_LED() {
			std::fill_n(timeout, 8, 0);
			std::fill_n(gpio, 8, GPIOD);
			for(int i=0; i<8; ++i)
				pin[i]			= GPIO_PIN_0<<i;
		}

		void	poll(void) {
			for(int i = 0; i < sizeof(pin)/sizeof(uint32_t); ++i)
				if(HAL_GetTick() < timeout[i])	
					HAL_GPIO_WritePin(gpio[i],pin[i], GPIO_PIN_RESET);
				else
				HAL_GPIO_WritePin(gpio[i],pin[i], GPIO_PIN_SET);
		}
		void RED1(int32_t t)			{ timeout[0]=t+HAL_GetTick(); };
		void RED2(int32_t t)			{ timeout[4]=t+HAL_GetTick(); };
		void GREEN1(int32_t t)		{ timeout[1]=t+HAL_GetTick(); };
		void GREEN2(int32_t t)		{ timeout[5]=t+HAL_GetTick(); };
		void YELLOW1(int32_t t)		{ timeout[2]=t+HAL_GetTick(); };
		void YELLOW2(int32_t t) 	{ timeout[6]=t+HAL_GetTick(); };
		void BLUE1(int32_t t)			{ timeout[3]=t+HAL_GetTick(); };
		void BLUE2(int32_t t)			{ timeout[7]=t+HAL_GetTick(); };
};

//_____________________________________________________________________
typedef enum {_OFF,_1,_2,_3,_4} _Footsw;
#define	_FOOT_MASK 0x000f

class _FOOTSW {
	private:
		int temp,timeout;
	public:
		_FOOTSW() {
			temp=timeout=0;
		}
		
	int poll(__packed _Footsw *f) {
		_Footsw key[]={_OFF,_OFF,_OFF,_3,_OFF,_OFF,_OFF,_4,_OFF,_OFF,_1,_2,_OFF,_OFF,_OFF,_OFF};
		
			if(temp != (GPIOE->IDR & _FOOT_MASK)) {
				temp = GPIOE->IDR & _FOOT_MASK;
				timeout = HAL_GetTick() + 2;
			} else 
					if(timeout && HAL_GetTick() > timeout) {
						timeout=0;
						if(key[temp] != *f) {
							*f=key[temp];
							return *f;
						}
			}
			return EOF;
}
};

#endif
