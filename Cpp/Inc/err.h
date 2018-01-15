#ifndef ERR_H
#define ERR_H
#include 	"stm32f4xx_hal.h"
//_____________________________________________________________________
typedef	enum {

	_NOERR						=0,
	_V5								=(1<<0),
	_V12							=(1<<1),
	_V24							=(1<<2),
	_sprayInPressure	=(1<<3),
	
	_sysOverheat			=(1<<4),
	_pumpTacho				=(1<<5),
	_pumpPressure			=(1<<6),
	_pumpCurrent			=(1<<7),
	
	_fan1Tacho				=(1<<8),
	_emgDisabled			=(1<<9),
	_handpcDisabled		=(1<<10),
	_illstatereq			=(1<<11),
	
	_energy_missing		=(1<<12),
	_sprayNotReady		=(1<<13),
	_doorswDisabled		=(1<<14),
	_fan2Tacho				=(1<<15)
} _err;         

inline _err operator~(_err a)
{return static_cast<_err>(~static_cast<int>(a));}
inline _err operator |  (_err a, _err b)	{return static_cast<_err>(static_cast<int>(a) | static_cast<int>(b));}
inline _err operator &  (_err a, _err b)	{return static_cast<_err>(static_cast<int>(a) & static_cast<int>(b));}
inline _err operator ^	(_err a, _err b)	{return static_cast<_err>(static_cast<int>(a) ^ static_cast<int>(b));}

#define _PUMP_ERR_DELAY		5000
#define _FAN_ERR_DELAY		5000
#define _TACHO_ERR_DELAY	100
#define _EC20_EM_DELAY		5

#define	_12Voff_ENABLE		HAL_GPIO_WritePin(GPIOB,GPIO_Pin_3, GPIO_PIN_RESET)
#define	_12Voff_DISABLE		HAL_GPIO_WritePin(GPIOB,GPIO_Pin_3, GPIO_PIN_SET)

#define	_SYS_SHG_ENABLE		HAL_GPIO_WritePin(_SYS_SHG_GPIO_Port,_SYS_SHG_Pin, GPIO_PIN_RESET)
#define	_SYS_SHG_DISABLE	HAL_GPIO_WritePin(_SYS_SHG_GPIO_Port,_SYS_SHG_Pin, GPIO_PIN_SET)
#define	_SYS_SHG_ENABLED	!HAL_GPIO_ReadPin(_SYS_SHG_GPIO_Port,_SYS_SHG_Pin)

#define	_cwbBUTTON				HAL_GPIO_ReadPin(cwbBUTTON_GPIO_Port,cwbBUTTON_Pin)
#define	_cwbDOOR					HAL_GPIO_ReadPin(cwbDOOR_GPIO_Port,cwbDOOR_Pin)
#define	_cwbENGM					HAL_GPIO_ReadPin(cwbENGM_GPIO_Port,cwbENGM_Pin)

#define	_EMG_DISABLED			HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_8)

#endif