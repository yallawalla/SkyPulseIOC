#include "stm32f4xx_hal.h"
#include "ioc.h"

extern "C" {
	void ioc(void) {
		_IOC::parent=new _IOC;
		_task(NULL);
	}
}
_IOC*	_IOC::parent			= NULL;
/*******************************************************************************
* Function Name	:
* Description		:
* Output				:
* Return				:
*******************************************************************************/
_IOC::_IOC() : can(&hcan2),com1(&huart1),com3(&huart3) {
	error_mask = warn_mask = _sprayInPressure | _sprayNotReady;
	dbgio=NULL;
	
	FIL f;
	if(f_open(&f,"0:/lm.ini",FA_READ) == FR_OK) {
		pump.LoadSettings(&f);
		fan.LoadSettings(&f);
		spray.LoadSettings(&f);
		ws2812.LoadSettings(&f);
		f_close(&f);
	}	else
		_print("... error settings file");
	

	_proc_add((void *)pollStatus,this,(char *)"error task",1);
	_proc_add((void *)taskRx,this,(char *)"can rx",0);
	ws2812.Batch((char *)"onoff.led");
	SetState(_STANDBY);	
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
void	*_IOC::pollStatus(void *v) {
_IOC *me=static_cast<_IOC *>(v);
			me->SetError(me->pump.Status());
			me->SetError(me->fan.Status());
			me->SetError(me->spray.Status());
			me->SetError(me->adcError());
			return me;
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
					ws2812.Batch((char *)"standby.led");
					_SYS_SHG_ENABLE;
					break;
				case	_READY:
					IOC_State.State = _READY;
					pump.Enable();
					ws2812.Batch((char *)"ready.led");
					break;
				case	_ACTIVE:
					IOC_State.State = _ACTIVE;
					pump.Enable();
					ws2812.Batch((char *)"active.led");
					break;
				case	_ERROR:
					IOC_State.State = _ERROR;
					ws2812.Batch((char *)"error.led");
					_SYS_SHG_DISABLE;
					break;
				default:
					break;
				}
}
/*******************************************************************************
* Function Name	:
* Description		:
* Output				:
* Return				:
*******************************************************************************/
void	_IOC::SetError(_err err) {
int		w = (err ^ IOC_State.Error) & err & warn_mask;
int		e = (err ^ IOC_State.Error) & err & ~error_mask;
			if(__time__ > 3000) {
				if(e) {
					_SYS_SHG_DISABLE;
					if(e & (_pumpCurrent | _flowTacho))
						pump.Disable();
					if(IOC_State.State != _ERROR)
						ws2812.Batch((char *)"error.led");
					IOC_State.State = _ERROR;
				}
				if(e | w) {
					IOC_State.Error = (_err)(IOC_State.Error | e | w );
					IOC_State.Send();
				}

				if(_SYS_SHG_ENABLED)
					__GREEN2(200);
				else
					__RED2(200);


				if((e | w) && dbgio && (debug & DBG_ERR)) {
					_io *temp=_stdio(dbgio);
					for(int n=0; n<32; ++n)
						if(e & (1<<n))
							_print("\r\nerror   %04d: %s",n, ErrMsg[n].c_str());	
					for(int n=0; n<32; ++n)
						if(w & (1<<n))
							_print("\r\nwarning %04d: %s",n, ErrMsg[n].c_str());
					_stdio(temp);
				} 	
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
	"pump pressure out of range",
	"pump current out of range",
	
	"fan rate out of range",
	"emergency button pressed",
	"handpiece crowbar fail",
	"flow rate out of range",
	
	"energy report timeout",
	"spray not ready",
	"doorswitch crowbar fail"
};
