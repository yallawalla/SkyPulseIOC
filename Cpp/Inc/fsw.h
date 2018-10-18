#ifndef		FSW_H
#define		FSW_H
#include	"misc.h"
/**
  ******************************************************************************
  * @file    fsw.h
  * @brief	 _FSW class definition
  *
  */
/** @addtogroup ootswitch
* @{
*/
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
	typedef enum {
	__FSW_OFF	=((FSW3_Pin | FSW2_Pin | FSW0_Pin)	<<8),
	__FSW_1		=((											 FSW0_Pin)	<<8),
	__FSW_2		=((FSW3_Pin 					 | FSW0_Pin)	<<8),
	__FSW_3		=((FSW3_Pin 										 )	<<8),
	__FSW_4		=((FSW3_Pin | FSW2_Pin 					 )	<<8)
} __FOOT;
	
#define __FSW (((FSW0_GPIO_Port->IDR & FSW0_Pin) |  \
					(FSW2_GPIO_Port->IDR & FSW2_Pin) | 				\
						(FSW3_GPIO_Port->IDR & FSW3_Pin))<<8)
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
class _FSW {
	private:
		int key,temp,timeout;
	public:
		_FSW();
		int Read(void);
	};
/**
  * @}
  */

#endif
