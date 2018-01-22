#ifndef ERR_H
#define ERR_H
#include 	"stm32f4xx_hal.h"
//_____________________________________________________________________
typedef	enum {
	_NOERR						=0,
	_V5								=0x0001,
	_V12							=0x0002,
	_V24							=0x0004,
	_sprayInPressure	=0x0008,
	_sysOverheat			=0x0010,
	_pumpTacho				=0x0020,
	_pumpPressure			=0x0040,
	_pumpCurrent			=0x0080,
	_fan1Tacho				=0x0100,
	_emgDisabled			=0x0200,
	_handpcDisabled		=0x0400,
	_fan2Tacho				=0x0800,
	_energyMissing		=0x1000,
	_sprayNotReady		=0x2000,
	_doorswDisabled		=0x4000
} _err;           
           
inline _err operator~(_err a)
{return static_cast<_err>(~static_cast<int>(a));}
inline _err operator |  (_err a, _err b)	{return static_cast<_err>(static_cast<int>(a) | static_cast<int>(b));}
inline _err operator &  (_err a, _err b)	{return static_cast<_err>(static_cast<int>(a) & static_cast<int>(b));}
inline _err operator ^	(_err a, _err b)	{return static_cast<_err>(static_cast<int>(a) ^ static_cast<int>(b));}

#define _PUMP_ERR_DELAY		100
#define _FAN_ERR_DELAY		100
#define _TACHO_ERR_DELAY	100
#define _EC20_EM_DELAY		5

#define	_12Voff_ENABLE		HAL_GPIO_WritePin(GPIOB,GPIO_Pin_3, GPIO_PIN_RESET)
#define	_12Voff_DISABLE		HAL_GPIO_WritePin(GPIOB,GPIO_Pin_3, GPIO_PIN_SET)

#define	_SYS_SHG_ENABLE		HAL_GPIO_WritePin(_SYS_SHG_GPIO_Port,_SYS_SHG_Pin, GPIO_PIN_SET)
#define	_SYS_SHG_DISABLE	HAL_GPIO_WritePin(_SYS_SHG_GPIO_Port,_SYS_SHG_Pin, GPIO_PIN_RESET)
#define	_SYS_SHG_ENABLED	HAL_GPIO_ReadPin(_SYS_SHG_GPIO_Port,_SYS_SHG_Pin)

#define	_cwbBUTTON				HAL_GPIO_ReadPin(cwbBUTTON_GPIO_Port,cwbBUTTON_Pin)
#define	_cwbDOOR					HAL_GPIO_ReadPin(cwbDOOR_GPIO_Port,cwbDOOR_Pin)
#define	_cwbENGM					HAL_GPIO_ReadPin(cwbENGM_GPIO_Port,cwbENGM_Pin)

#define	_EMG_DISABLED			HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_8)

#endif
