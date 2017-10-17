#include "footsw.h"
uint32_t			_LED::timeout[]	={0,0,0,0,0,0,0,0};
uint32_t			_LED::pin[]			={GPIO_PIN_0,GPIO_PIN_1,GPIO_PIN_2,GPIO_PIN_3,GPIO_PIN_4,GPIO_PIN_5,GPIO_PIN_6,GPIO_PIN_7};
GPIO_TypeDef*	_LED::gpio[]		={GPIOD,GPIOD,GPIOD,GPIOD,GPIOD,GPIOD,GPIOD,GPIOD};

void					_LED::poll(void *v) {
							for(int i = 0; i < sizeof(pin)/sizeof(uint32_t); ++i)
								if(HAL_GetTick() < timeout[i])
									HAL_GPIO_WritePin(gpio[i],pin[i], GPIO_PIN_RESET);
								else
									HAL_GPIO_WritePin(gpio[i],pin[i], GPIO_PIN_SET);
}


/*******************************************************************************
* Function Name	: Poll()
* Description		:	footswitch port polling
* Output				: int
* Return				: footswitch code, 20ms filter, on valid change
*******************************************************************************/
int   _GPIO::Poll(void) {
			if(temp != (GPIOE->IDR & _FOOT_MASK)) {
				temp = GPIO_ReadInputData(GPIOC) & _FOOT_MASK;
				timeout = __time__ + 5;
			} else 
					if(timeout && __time__ > timeout) {
						timeout=0;
						if(temp != key) {
							key=temp;
							return key;
						}
			}
			return EOF;
}