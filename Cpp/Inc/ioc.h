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
// inputs from SYS, filter 1
	idIOC_State				=0x200,			// state request
	idIOC_SprayParm		=0x201,			// spray parameters
	idIOC_Footreq			=0x202,			// footswitch request
	idIOC_AuxReq			=0x203,			// debug request
	idIOC_VersionReq	=0x204,			// sw version
	idDL_sTestReq			=0x205,			// diode selftest
	idCAN2FOOT				=0x20C,			// send to fsw port
	idCAN2COM					=0x20B,			// send to console port
// ack. outputs to SYS, filter 
	idIOC_State_Ack		=0x240,
	idIOC_FootAck			=0x241,
	idIOC_SprayAck		=0x242,
	idIOC_AuxAck			=0x243,
	idIOC_VersionAck	=0x244,
	idDL_sTestAck			=0x245,
  idCOM2CAN					=0x24B,
	idFOOT2CAN				=0x24C,
// input from DL
	idDL_Limits				=0x21F,			// DL ports limit parameters
	idDL_State				=0x601,			// DL state req.(from SYS)			
	idDL_Params				=0x602,			// DL pulse parameters(from SYS)			
// inputs EC20
	idEC20_req				=0x280,			// laser announce
	idEC20_Params			=0x103,			// EC20 pulse parameters(from SYS)
// inputs ENM
	idEM_ack					=0x0C0,			// ENM pulse acknowledge 
// misc (from SYS)
  idBOOT						=0x20				// BOOT request
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

typedef enum {
	_DIODE_NOT_READY,
	_DIODE_READY
} _DIODE;
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
typedef __packed struct _DL_sTestAck {
	_DIODE	Status;
	void	Send() {
		_CAN::Send(idDL_sTestAck,(void *)&Status,sizeof(_DL_sTestAck));
	}
} DL_sTestAck;
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
		_DL_sTestAck		DL_sTestAck;

		void						LoadSettings();
		void						SaveSettings();
	
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

		_err fswStatus(void);
		
		void SetState(uint8_t *,uint8_t);
		void SetState(_State);
		void pollError();

		static const string	ErrMsg[];
		static void	*pollStatus(void *);
		static void	taskRx(_IOC *me) {
			me->can.pollRx(me);
		}
};
#endif
