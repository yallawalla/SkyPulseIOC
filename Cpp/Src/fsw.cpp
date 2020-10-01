/**
  ******************************************************************************
  * @file    fsw.cpp
  * @brief	 _FSW class members
  *
  */
/** @addtogroup footswitch
* @{
*/
#include "fsw.h"
#include "err.h"
#include "misc.h"
/*******************************************************************************
* Function Name	:
* Description		:
* Output				:
* Return				:
*******************************************************************************/
_FSW::_FSW(void) {
			timeout=error_timeout=0;
			key=__FSW_OFF;			
}
/*******************************************************************************
* Function Name	:
* Description		:
* Output				:
* Return				:
*******************************************************************************/
int		_FSW::Read() {	
			int i=__FSW;

			if(i != temp) {
				temp = i;
				timeout = __time__ + 5;
			} else 
					if(timeout && __time__ > timeout) {
						timeout=0;
						if(temp != key ) {
							key=temp ;
							return (key);
						}
			}
			return EOF;
}
/*******************************************************************************
* Function Name	:
* Description		:
* Output				:
* Return				:
*******************************************************************************/
int 	_FSW::sTest(int k) {
	if(k != EOF) {
		GPIO_InitTypeDef GPIO_InitStruct = {0};
		HAL_GPIO_WritePin(FSW0_GPIO_Port, FSW2_Pin|FSW3_Pin|FSW0_Pin|FSW1_Pin, GPIO_PIN_SET);
		GPIO_InitStruct.Pin = FSW2_Pin|FSW3_Pin|FSW0_Pin|FSW1_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		HAL_GPIO_Init(FSW0_GPIO_Port, &GPIO_InitStruct);
		FSW0_GPIO_Port->ODR &= (~(FSW2_Pin|FSW3_Pin|FSW0_Pin|FSW1_Pin) | k);
	}
	return FSW0_GPIO_Port->IDR & (FSW2_Pin|FSW3_Pin|FSW0_Pin|FSW1_Pin);
}
/**
* @}
*/ 


