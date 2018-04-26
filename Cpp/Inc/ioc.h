#ifndef IOC_H
#define IOC_H

#include "stm32f4xx_hal.h"
#include "can.h"
#include "cli.h"
#include "adc.h"
#include "misc.h"
#include "err.h"
#include "fan.h"
#include "pump.h"
#include "spray.h"
#include "rtc.h"
#include "ws2812.h"

#include <string>
#include <ctype.h>

typedef enum {
	DBG_OFF						=0,
	DBG_CAN_TX				=1<<0,
	DBG_CAN_RX				=1<<1,
	DBG_ERR						=1<<2,
	DBG_INFO					=1<<3,
	DBG_CAN_COM				=1<<21,
	DBG_EC_SIM				=1<<22,
	DBG_ENRG					=1<<23
}	_dbg;

typedef enum {    
	idIOC_State				=0x200,
	idIOC_SprayParm		=0x201,
	idIOC_Footreq			=0x202,
	idIOC_AuxReq			=0x203,
	idIOC_State_Ack		=0x240,
	idIOC_FootAck			=0x241,
	idIOC_SprayAck		=0x242,
	idIOC_AuxAck			=0x243,
	idCAN2COM					=0x20B,
  idCOM2CAN					=0x24B,
	idCAN2FOOT				=0x20C,
	idFOOT2CAN				=0x24C,
	idEC20_req				=0x280,
	idEM_ack					=0x0C0,
  idBOOT						=0x20
} _StdId;

typedef enum {
	_STANDBY,
	_READY,
	_ACTIVE,
	_ERROR
} _State;

typedef enum {
	_OFF,
	_1,
	_2,
	_3,
	_4		
} _Footsw;

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
		_CAN::Send(idIOC_State_Ack,(void *)&State,sizeof(_IOC_State));
	}
} IOC_State;
//_____________________________________________________________________
typedef __packed struct _IOC_FootAck {
	_Footsw State;
	_IOC_FootAck() : State(_OFF)	{}	
	void	Send() {
		_CAN::Send(idIOC_FootAck,(void *)&State,sizeof(_IOC_FootAck));
	}
} IOC_FootAck;
//_____________________________________________________________________
typedef __packed struct _IOC_Aux{
	short Temp;
	_IOC_Aux() : Temp(0)	{}	
	void	Send() {
		_CAN::Send(idIOC_AuxAck,(void *)&Temp,sizeof(_IOC_Aux));
	}
} IOC_Aux;
//_____________________________________________________________________
typedef __packed struct _IOC_SprayAck {
	_Spray	Status;
	_IOC_SprayAck() : Status(_SPRAY_NOT_READY)	{}	
	void	Send() {
		_CAN::Send(idIOC_SprayAck,(void *)&Status,sizeof(_IOC_SprayAck));
	}
} IOC_SprayAck;
//_____________________________________________________________________
class _IOC : public _ADC {
	private:
		int key,temp;
	
	public:
		static _IOC			*parent;
		_IOC();
		_err 						error_mask,warn_mask;
		_dbg						debug;
		_io							*dbgio;
		_IOC_State 			IOC_State;
		_IOC_FootAck		IOC_FootAck;
		_IOC_SprayAck		IOC_SprayAck;
		_IOC_Aux				IOC_Aux;
		_CAN						can;
		_SPRAY 					spray;
		_WS 						ws2812;
		_PUMP 					pump;
		_FAN 						fan;
		_RTC						rtc;
		_CLI						com,com1,com3;

		~_IOC();

		void SetState(_State);
		void SetError(_err);

		static const string	ErrMsg[];
		static void	*pollStatus(void *);
		static void	taskRx(_IOC *me) {
			me->can.pollRx(me);
		}
};
#endif
