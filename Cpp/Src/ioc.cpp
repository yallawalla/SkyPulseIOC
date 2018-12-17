#include "stm32f4xx_hal.h"
#include "ioc.h"
#include "io.h"
extern "C" {
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
****************************f***************************************************/
void	makeIoc(void) {
			_IOC::parent=new _IOC;
			
			_stdio(_IOC::parent->com1.io);
			if(__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST))
				_print("\rSWR");
			else if(__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST))
				_print("\rIWDG");
			else if(__HAL_RCC_GET_FLAG(RCC_FLAG_WWDGRST))
				_print("\rWWDG");
			else if(__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST))
				_print("\rPOR");
			else if(__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST))
			{} else {}
			__HAL_RCC_CLEAR_RESET_FLAGS();
				
			_print(" reset, CPU %dMHz\r\nV",SystemCoreClock/1000000);
			 printVersion();
			_print("\r\n/");
				
			_stdio(NULL);
			}
}
_IOC*	_IOC::parent			= NULL;
/*******************************************************************************
* Function Name	:
* Description		:
* Output				:
* Return				:
*******************************************************************************/
_IOC::_IOC() : can(&hcan2),com1(&huart1),com3(&huart3),comUsb(&hUsbDeviceFS) {
			parent=this;
			error_mask = warn_mask = _sprayInPressure | _sprayNotReady;
			SetState(_STANDBY);	
			
			FIL f;
			if(f_open(&f,"0:/ioc.ini",FA_READ) == FR_OK) {
char		c[128];
				pump.LoadSettings(&f);
				fan.LoadSettings(&f);
				spray.LoadSettings(&f);
				ws2812.LoadSettings(&f);
				if(f_gets(c,sizeof(c),&f))
					sscanf(c,"%X,%X",&error_mask,&warn_mask);
				while(!f_eof(&f))
					com1.Parse(&f);	
				f_close(&f);
			}	else
				_print("... error settings file");
			
			_proc_add((void *)pollStatus,this,(char *)"error task",1);
			_proc_add((void *)taskRx,this,(char *)"can rx",0);
			
			int	i=0;
			while(strncmp(Months[i++],__DATE__,3))
				if(i == 12)
					break;
				
			IOC_VersionAck.hash=HAL_CRC_Calculate(&hcrc,__Vectors, (FATFS_ADDRESS-(int)__Vectors)/sizeof(int));
			IOC_VersionAck.date=atoi(&__DATE__[4]);
			IOC_VersionAck.month=i + 12*(atoi(&__DATE__[7])-2018);
						
			ws2812.Batch((char *)"@onoff.ws");
}
/*******************************************************************************
* Function Name	:
* Description		:
* Output				:
* Return				:
*******************************************************************************/
_IOC::~_IOC() {
	
}
/*******************************************************************************
* Function Name	:
* Description		:
* Output				:
* Return				:
*******************************************************************************/
_err	_IOC::fswError() {
			switch(Fsw.Read()) {
				case EOF:
					break;
				case __FSW_OFF:
					IOC_FootAck.State=_OFF;
					IOC_FootAck.Send();
					_TERM::Debug(DBG_INFO,"\r\n:\r\n:footswitch disconnected \r\n:");					
					break;
				case __FSW_1:
					IOC_FootAck.State=_1;
					IOC_FootAck.Send();
					_TERM::Debug(DBG_INFO,"\r\n:\r\n:footswitch state 1\r\n:");					
					break;
				case __FSW_2:
					IOC_FootAck.State=_2;
					IOC_FootAck.Send();
					_TERM::Debug(DBG_INFO,"\r\n:\r\n:footswitch state 2\r\n:");					
					break;
				case __FSW_3:
					IOC_FootAck.State=_3;
					IOC_FootAck.Send();
					_TERM::Debug(DBG_INFO,"\r\n:\r\n:footswitch state 3\r\n:");					
					break;
				case __FSW_4:
					IOC_FootAck.State=_4;
					IOC_FootAck.Send();
					_TERM::Debug(DBG_INFO,"\r\n:\r\n:footswitch state 4\r\n:");										
					break;
				default:
					_TERM::Debug(DBG_INFO,"\r\n:\r\n:footswitch error\r\n:");		
					return _footswError;			
			}
			return _NOERR;
}
/*******************************************************************************
* Function Name	:
* Description		:
* Output				:
* Return				:
*******************************************************************************/
void	*_IOC::pollStatus(void *v) {
_IOC	*ioc = static_cast<_IOC *>(v);
			ioc->pollError();
			return v;
}
/*******************************************************************************
* Function Name	:
* Description		:
* Output				:
* Return				:
*******************************************************************************/
void	_IOC::pollError() {
_err	err = can.Status();
			err = err | pump.Status();
			err = err | fan.Status();
			err = err | spray.Status();
			err = err | fswError();
			err = err | diode.Status(IOC_State.State == _ACTIVE);
	
_err	w = (err ^ IOC_State.Error) & warn_mask;
_err	e = (err ^ IOC_State.Error) & err & ~error_mask;

			if(__time__ > 3000 && (e | w)) {
			
				IOC_State.Error = (IOC_State.Error | e) ^ w ;
				IOC_State.Send();
				
				if(e)
					SetState(_ERROR);

				for(int n=0; n<32; ++n)
					if(e & (1<<n))
						_TERM::Debug(DBG_ERR,"\r\nerror   %04d: %s",n, ErrMsg[n].c_str());	
				for(int n=0; n<32; ++n)
					if(w & (1<<n)) {
						if(w & IOC_State.Error)
							_TERM::Debug(DBG_ERR,"\r\nwarning %04d: %s",n, ErrMsg[n].c_str());
						else
							_TERM::Debug(DBG_ERR,"\r\nwarning %04d: ...",n);
					}
			}
			
			_SYS_SHG_ENABLED ? __GREEN2(200) : __RED2(200);
}
/*******************************************************************************
* Function Name	:
* Description		:
* Output				:
* Return				:
*******************************************************************************/
void	_IOC::SetState(uint8_t *data) {
			pump.mode &= ~_PUMP_BOOST;
			fan.mode &= ~(_FAN_BOOST0 | _FAN_BOOST1);
			SetState((_State)data[0]);
	
			pump.mode |= data[1] & _PUMP_BOOST;
			fan.mode |= data[1] & (_FAN_BOOST0 | _FAN_BOOST1);
}
/*******************************************************************************
* Function Name	:
* Description		:
* Output				:
* Return				:
*******************************************************************************/
void	_IOC::SetState(_State s) {
			switch(s) {
				case	_STANDBY:
					if(IOC_State.State == _ERROR)
						IOC_State.Error = _NOERR;
					IOC_State.State = _STANDBY;
					pump.Enable();
					ws2812.Batch((char *)"@standby.ws");
					_SYS_SHG_ENABLE;
					break;
				case	_READY:
					IOC_State.State = _READY;
					pump.Enable();
					ws2812.Batch((char *)"@ready.ws");
					break;
				case	_ACTIVE:
					IOC_State.State = _ACTIVE;
					pump.Enable();
					ws2812.Batch((char *)"@active.ws");
					break;
				case	_ERROR:
					if(IOC_State.State != _ERROR) {
						IOC_State.State = _ERROR;
						ws2812.Batch((char *)"@error.ws");
						_SYS_SHG_DISABLE;	
						spray.AirLevel=spray.WaterLevel=0;
						if(IOC_State.Error & (_pumpCurrent | _flowTacho))
							pump.Disable();
					}
					break;
				case	_CALIBRATE:
					break;
			}
}
/*******************************************************************************
* Function Name	:
* Description		:
* Output				:
* Return				:
*******************************************************************************/
const string _IOC::ErrMsg[] = {
			"5V  supply",
			"12V supply",
			"24V supply",
			"spray input pressure",
			
			"cooler temperature",
			"pump rate out of range",
			"DL timeout",
			"pump current out of range",
			
			"fan rate out of range",
			"emergency button pressed",
			"handpiece crowbar fail",
			"flow rate out of range",
			
			"ENM timeout",
			"spray not ready",
			"doorswitch crowbar fail",
			"footswitch error",

			"illegal EC20 req.",
			"illegal ENM ack",
			"DL power, channel 1",
			"DL power, channel 2",
			
			"Temp. sensor error"
};
