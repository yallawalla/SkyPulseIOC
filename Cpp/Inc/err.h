#ifndef ERR_H
#define ERR_H
#include 	"stm32f4xx_hal.h"
//_____________________________________________________________________
typedef	enum {
	_NOERR								=0,
	_V5										=1<<0,
	_V12									=1<<1,
	_V24									=1<<2,
	_sprayInPressure			=1<<3,
	_sysOverheat					=1<<4,
	_pumpTacho						=1<<5,
	_pumpPressure					=1<<6,
	_pumpCurrent					=1<<7,
	_fan1Tacho						=1<<8,
	_fan2Tacho						=1<<9,
	_emgDisabled					=1<<10,
	_pyroNoresp						=1<<11,
	_illstatereq					=1<<12,
	_energy_missing				=1<<13
} _Error;           		

#define _PUMP_ERR_DELAY		5000
#define _FAN_ERR_DELAY		5000
#define _TACHO_ERR_DELAY	100
#define _EC20_EM_DELAY		5

extern	uint32_t pump_cbk, fan1_cbk, fan2_cbk;

#endif
