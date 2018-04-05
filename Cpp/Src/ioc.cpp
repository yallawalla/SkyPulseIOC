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
	
	SetState(_STANDBY);	
	error_mask = _flowTacho | _sprayInPressure | _sprayNotReady;
	
	_proc_add((void *)pollStatus,this,(char *)"error task",1);
	
	FIL f;
	if(f_open(&f,"0:/lm.ini",FA_READ) == FR_OK) {
		pump.LoadSettings(&f);
		fan.LoadSettings(&f);
		spray.LoadSettings(&f);
		ws2812.LoadSettings(&f);
		f_close(&f);
	}	else
		_print("... error settings file");
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
//			me->adcFilter();
			me->SetError(me->pump.Status());
			me->SetError(me->fan.Status());
			me->SetError(me->spray.Status());
			me->SetError(me->adcError());
			me->led.poll();
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
						IOC_State.State = _STANDBY;
						IOC_State.Error = _NOERR;
						//Submit("@standby.led");
						_SYS_SHG_ENABLE;
						break;
					case	_READY:
							IOC_State.State = _READY;
							//Submit("@ready.led");
						break;
					case	_ACTIVE:
							IOC_State.State = _ACTIVE;
							//Submit("@active.led");
						break;
					case	_ERROR:
						IOC_State.State = _ERROR;
						//Submit("@error.led");
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
void	_IOC::SetError(_err e) {
	
int		ee = (e ^ IOC_State.Error) & e & ~error_mask;
	
			if(_SYS_SHG_ENABLED)
				led.GREEN1(20);
			else
				led.RED1(20);
			
			if(ee && __time__ > 3000) {
				_SYS_SHG_DISABLE;
//				if(IOC_State.State != _ERROR)
//					Submit("@error.led");
				IOC_State.Error = (_err)(IOC_State.Error | ee);
				IOC_State.State = _ERROR;
				IOC_State.Send();
			} 

//int		ww=(e ^ IOC_State.Error) & warn_mask;
//			if(ww && __time__ > 3000) {
//				IOC_State.Error = (_Error)(IOC_State.Error ^ ww);
//				IOC_State.Send();
//			} 
			

			for(int n=0; e && debug & (1<<DBG_ERR); e = (_err)(e>>1), ++n)
				if(e & (1<<0))
					_print("\r\nerror %03d: %s",n, ErrMsg[n].c_str());	
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
