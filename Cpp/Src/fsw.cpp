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
#include "misc.h"
/*******************************************************************************
* Function Name	:
* Description		:
* Output				:
* Return				:
*******************************************************************************/
_FSW::_FSW(void) {
			timeout=0;
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
				timeout = __time__ + 10;
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

/**
* @}
*/ 


