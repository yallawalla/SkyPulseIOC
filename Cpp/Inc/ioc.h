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
#include "dl.h"
#include "ws2812.h"
#include <string>
#include <ctype.h>

typedef enum {    
	idIOC_State				=0x200,
	idIOC_SprayParm		=0x201,
	idIOC_Footreq			=0x202,
	idIOC_AuxReq			=0x203,
	idIOC_VersionReq	=0x204,
	idDL_Limits				=0x21F,
	idDL_State				=0x601,
	idDL_Timing				=0x602,
	idIOC_State_Ack		=0x240,
	idIOC_FootAck			=0x241,
	idIOC_SprayAck		=0x242,
	idIOC_AuxAck			=0x243,
	idIOC_VersionAck	=0x244,
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
	_ERROR,
	_CALIBRATE
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
typedef __packed struct _IOC_SprayAck {
	_Spray	Status;
	_IOC_SprayAck() : Status(_SPRAY_NOT_READY)	{}	
	void	Send() {
		_CAN::Send(idIOC_SprayAck,(void *)&Status,sizeof(_IOC_SprayAck));
	}
} IOC_SprayAck;
//_____________________________________________________________________
typedef __packed struct _IOC_VersionAck {
	uint16_t version;
	uint32_t hash;
	uint8_t	 date;
	uint8_t  month;
	
	_IOC_VersionAck() : version(SW_version),hash(0),date(0),month(0)	{}	
	void	Send() {
		_CAN::Send(idIOC_VersionAck,(void *)&version,sizeof(_IOC_VersionAck));
	}
} IOC_VersionAck;
//_____________________________________________________________________
typedef __packed struct _IOC_Aux{
	uint16_t	Temp;
	uint8_t		Flow;
	uint8_t		Pump;
	uint8_t		Fan;
	_IOC_Aux() : Temp(0),Flow(0),Pump(0),Fan(0)	{}	
	void	Send() {
		_CAN::Send(idIOC_AuxAck,(void *)&Temp,sizeof(_IOC_Aux));
	}
} IOC_Aux;
//_____________________________________________________________________
typedef __packed struct _DL_State {
	_State 	State;
} DL_State;
//_____________________________________________________________________
typedef __packed struct _DL_Timing {
	unsigned short Pavg;
	unsigned Ton:24,Toff:24;
} DL_Timing;
//_____________________________________________________________________
typedef __packed struct _DL_Limits {
	unsigned short L0,L1;
	unsigned char mode;
} DL_Limits;
//_____________________________________________________________________
class _IOC {
	private:
		int key,temp;
	
	public:
		static _IOC			*parent;
		_IOC();
		_err 						error_mask,warn_mask;
	
		_IOC_State 			IOC_State;
		_IOC_FootAck		IOC_FootAck;
		_IOC_SprayAck		IOC_SprayAck;
		_IOC_Aux				IOC_Aux;
		_IOC_VersionAck	IOC_VersionAck;
	
		_CAN						can;
		_SPRAY 					spray;
		_WS 						ws2812;
		_PUMP 					pump;
		_FAN 						fan;
		_DL							diode;
		_RTC						rtc;
		_FSW						Fsw;
		_CLI						com1,com3,comUsb;
		~_IOC();

		void SetState(uint8_t *);
		void SetState(_State);
		void pollError();

		static const string	ErrMsg[];
		static void	*pollStatus(void *);
		static void	taskRx(_IOC *me) {
			me->can.pollRx(me);
		}
		_err fswError(void);
};
#endif
