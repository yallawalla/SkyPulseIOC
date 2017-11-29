#ifndef IOC_H
#define IOC_H
#include "stm32f4xx_hal.h"
#include "can.h"
#include "cli.h"
#include "adc.h"
#include "leds.h"
#include "misc.h"
#include "err.h"
#include "fan.h"
#include "pump.h"
#include "spray.h"
#include "rtc.h"
#include "ws2812.h"

#include <string>
#include <ctype.h>

#define	_12Voff_ENABLE		HAL_GPIO_WritePin(GPIOB,GPIO_Pin_3, GPIO_PIN_RESET)
#define	_12Voff_DISABLE		HAL_GPIO_WritePin(GPIOB,GPIO_Pin_3, GPIO_PIN_SET)

#define	_SYS_SHG_ENABLE		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4, GPIO_PIN_SET)
#define	_SYS_SHG_DISABLE	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4, GPIO_PIN_RESET)
#define	_SYS_SHG_ENABLED	HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_4)

#define	_EMG_DISABLED			HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_8)

typedef enum {
	DBG_OFF			=0,
	DBG_CAN_TX	=1<<0,
	DBG_CAN_RX	=1<<1,
	DBG_ERR			=1<<2,
	DBG_INFO		=1<<3,
	DBG_CAN_COM	=1<<21,
	DBG_EC_SIM	=1<<22,
	DBG_ENRG		=1<<23
}	_dbg;

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
	idCOM2CAN							=0x20B,
  idCAN2COM							=0x24B,
	idCAN2FOOT						=0x20C,
	idFOOT2CAN						=0x24C,
	idEC20_req						=0x280,
	idEM_ack							=0x0C0,
  idBOOT								=0x20
} _StdId;
//_____________________________________________________________________
typedef enum {
	_STANDBY,
	_READY,
	_ACTIVE,
	_ERROR		
} _State;
//_____________________________________________________________________
typedef enum {
	_SPRAY_NOT_READY,
	_SPRAY_READY,
	_VIBRATE
} _Spray;
//_____________________________________________________________________
typedef __packed struct _IOC_State {
	_State 	State;
	_err	Error;	
	_IOC_State() : State(_STANDBY),Error(_NOERR)	{}
	void	Send() {
		CanTxMsgTypeDef	m={idIOC_State_Ack,0,CAN_ID_STD,CAN_RTR_DATA,sizeof(_IOC_State),0,0,0,0,0,0,0,0};
		memcpy(m.Data,(const void *)&State,sizeof(_IOC_State));
		_buffer_push(canBuffer->tx,&m,sizeof(CanTxMsgTypeDef));
	}
} IOC_State;
//_____________________________________________________________________
typedef __packed struct _IOC_FootAck {
	_Footsw State;
	_IOC_FootAck() : State(_OFF)	{}	
	void	Send() {
		CanTxMsgTypeDef	m={idIOC_FootAck,0,CAN_ID_STD,CAN_RTR_DATA,sizeof(_IOC_FootAck),0,0,0,0,0,0,0,0};
		memcpy(m.Data,(const void *)&State,sizeof(_IOC_FootAck));
		_buffer_push(canBuffer->tx,&m,sizeof(CanTxMsgTypeDef));
	}
} IOC_FootAck;
//_____________________________________________________________________
typedef __packed struct _IOC_SprayAck {
	_Spray	Status;
	_IOC_SprayAck() : Status(_SPRAY_NOT_READY)	{}	
	void	Send() {
		CanTxMsgTypeDef	m={idIOC_SprayAck,0,CAN_ID_STD,CAN_RTR_DATA,sizeof(_IOC_SprayAck),0,0,0,0,0,0,0,0};
		memcpy(m.Data,(const void *)&Status,sizeof(_IOC_SprayAck));
		_buffer_push(canBuffer->tx,&m,sizeof(CanTxMsgTypeDef));
	}
} IOC_SprayAck;
//_____________________________________________________________________
class _IOC : public _ADC {
	private:
		static const string	ErrMsg[];
	public:
		static _IOC			*parent;
		_IOC();
		_err 						error_mask;
		_dbg						debug;
		_IOC_State 			IOC_State;
		_IOC_FootAck		IOC_FootAck;
		_IOC_SprayAck		IOC_SprayAck;
		_CAN						can;
		_SPRAY 					spray;
		_WS 						ws2812;
		_PUMP 					pump;
		_FAN 						fan;
		_LED 						led;
		_RTC						rtc;
		_FOOTSW					footsw;
		_CLI						com,com1,com3;

		~_IOC();

		static void	*pollStatus(void *);
		void SetState(_State);
		void SetError(_err);
};
#endif
