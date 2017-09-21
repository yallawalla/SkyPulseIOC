#ifndef IOC_H
#define IOC_H
#include "stm32f4xx_hal.h"
#include "fs.h"
#include "can.h"
#include "leds.h"
#include "misc.h"

#include <string>
using namespace std;

#define	_ADC_ERR_DELAY	200
#define _PUMP_ERR_DELAY	3000
#define _FAN_ERR_DELAY	5000
#define _EC20_EM_DELAY	5

#define	_12Voff_ENABLE		HAL_GPIO_WritePin(GPIOB,GPIO_Pin_3, GPIO_PIN_RESET)
#define	_12Voff_DISABLE		HAL_GPIO_WritePin(GPIOB,GPIO_Pin_3, GPIO_PIN_SET)

#define	_SYS_SHG_ENABLE		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4, GPIO_PIN_SET)
#define	_SYS_SHG_DISABLE	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4, GPIO_PIN_RESET)
#define	_SYS_SHG_ENABLED	HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_4)

#define	_EMG_DISABLED			HAL_GPIO_ReadPin(GPIOA,GPIO_Pin_8)

typedef enum {
	DBG_OFF			=0,
	DBG_CAN_TX	=1<<0,
	DBG_CAN_RX	=1<<1,
	DBG_ERR			=1<<2,
	DBG_INFO		=1<<3,
	DBG_CAN_COM	=1<<21,
	DBG_EC_SIM	=1<<22,
	DBG_ENRG		=1<<23
}	_DEBUG_;

/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
typedef enum {    		  
	idIOC_State						=0x200,
	idIOC_SprayParm				=0x201,
	idIOC_SprayCmd				=0x202,
	idIOC_State_Ack				=0x240,
	idIOC_FootAck					=0x241,
	idIOC_SprayAck				=0x242,
	idCAN2COM							=0x20B,
  idCOM2CAN							=0x24B,
	idCAN2FOOT						=0x20C,
	idFOOT2CAN						=0x24C,
	idEC20_req						=0x280,
	idEM_ack							=0x0C0,
  idBOOT								=0x20
} _StdId;
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
	_fanTacho							=1<<8,
	_emgDisabled					=1<<9,
	_pyroNoresp						=1<<10,
	_illstatereq					=1<<11,
	_energy_missing				=1<<12
} _Error;           		
//_____________________________________________________________________
typedef enum {
	_STANDBY,
	_READY,
	_ACTIVE,
	_ERROR		
} _State;
//_____________________________________________________________________
typedef enum {
	_OFF,
	_1,
	_2,
	_3,
	_4		
} _Footsw;
//_____________________________________________________________________
typedef enum {
	_SPRAY_NOT_READY,
	_SPRAY_READY,
	_VIBRATE
} _Spray;
//_____________________________________________________________________
typedef __packed struct _IOC_State {
	_State 	State;
	_Error	Error;	
	_IOC_State() : State(_STANDBY),Error(_NOERR)	{}
	void	Send() {
		CanTxMsgTypeDef	m={idIOC_State_Ack,0,CAN_ID_STD,CAN_RTR_DATA,sizeof(_IOC_State),0,0,0,0,0,0,0,0};
		memcpy(m.Data,(const void *)&State,sizeof(_IOC_State));
		_CAN::instance->Send(&m);
	}
} IOC_State;
//_____________________________________________________________________
typedef __packed struct _IOC_FootAck {
	_Footsw State;
	_IOC_FootAck() : State(_OFF)	{}	
	void	Send() {
		CanTxMsgTypeDef	m={idIOC_FootAck,0,CAN_ID_STD,CAN_RTR_DATA,sizeof(_IOC_FootAck),0,0,0,0,0,0,0,0};
		memcpy(m.Data,(const void *)&State,sizeof(_IOC_FootAck));
		_CAN::instance->Send(&m);
	}
} IOC_FootAck;
//_____________________________________________________________________
typedef __packed struct _IOC_SprayAck {
	_Spray	Status;
	_IOC_SprayAck() : Status(_SPRAY_NOT_READY)	{}	
	void	Send() {
		CanTxMsgTypeDef	m={idIOC_SprayAck,0,CAN_ID_STD,CAN_RTR_DATA,sizeof(_IOC_SprayAck),0,0,0,0,0,0,0,0};
		memcpy(m.Data,(const void *)&Status,sizeof(_IOC_SprayAck));
		_CAN::instance->Send(&m);
	}
} IOC_SprayAck;
//_____________________________________________________________________
class _IOC {
	private:
		static	_Error 	error_mask;
		static  _DEBUG_	debug;
		static	string ErrMsg[];
	
	public:
		_IOC_State 		IOC_State;
		_IOC_FootAck	IOC_FootAck;
		_IOC_SprayAck	IOC_SprayAck;
		_FS						*com,*com1,*com3;
		_CAN					*can;
		_LED 					led;


_IOC();
~_IOC();
	static void	*poll(void *);
	void SetState(_State);
	void ErrParse(_Error);
};

#endif
