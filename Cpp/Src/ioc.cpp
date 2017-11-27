#include "stm32f4xx_hal.h"
#include "ioc.h"

extern "C" {
	void _p_loop(void);
		
	void ioc(void) {
		_IOC::parent=new _IOC;
		
		while(true) {
			_p_loop();
			vTaskDelay(1);
		}
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
	error_mask = _NOERR;
	
	_proc_add((void *)pollStatus,this,(char *)"error task",1);
	
	FIL f;
	if(f_open(&f,"0:/lm.ini",FA_READ) == FR_OK) {
		pump.LoadSettings(&f);
		fan.LoadSettings(&f);
		spray.LoadSettings(&f);
		ws2812.LoadSettings(&f);
		f_close(&f);
	}	else
		__print("... error settings file");
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
			me->adcSmooth();
			me->SetError(me->pump.Status());
			me->SetError(me->fan.Status());
			me->SetError(me->spray.Status());
			me->SetError(me->adcError());
	
			if(HAL_GetTick() > _TACHO_ERR_DELAY) {
				if(HAL_GetTick()-fan1_cbk > _FAN_ERR_DELAY)
					me->SetError(_fan1Tacho);
				if(HAL_GetTick()-fan2_cbk > _FAN_ERR_DELAY)
					me->SetError(_fan2Tacho);
			}
			if(_EMG_DISABLED && _SYS_SHG_ENABLED)
				me->SetError(_emgDisabled);
			
			me->led.poll();
			if(me->footsw.poll(&me->IOC_FootAck.State) != EOF)
				me->IOC_FootAck.Send();
			return NULL;
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
						if(IOC_State.State == _STANDBY || IOC_State.State == _ACTIVE) {
							IOC_State.State = _READY;
							//Submit("@ready.led");
						} else
							SetError(_illstatereq);
						break;
					case	_ACTIVE:
						if(IOC_State.State == _READY) {
							IOC_State.State = _ACTIVE;
							//Submit("@active.led");
						} else
							SetError(_illstatereq);
						break;
					case	_ERROR:
						IOC_State.State = _ERROR;
						//Submit("@error.led");
						_SYS_SHG_DISABLE;
						break;
					default:
						SetError(_illstatereq);
						break;
				}
}
/*******************************************************************************
* Function Name	:
* Description		:
* Output				:
* Return				:
*******************************************************************************/
extern int ttt;
void	_IOC::SetError(_err e) {

			e = (_err)(e & ~error_mask);
			ttt ? led.RED1(3000): led.GREEN1(20);
			e = (_err)((e ^ IOC_State.Error) & e);
	
			IOC_State.Error = (_err)(IOC_State.Error | e);

			if(e) {
				_SYS_SHG_DISABLE;
				IOC_State.State = _ERROR;
				IOC_State.Send();
//				if(IOC_State.State != _ERROR)
					//Submit("@error.led");
			}

			for(int n=0; e && debug & (1<<DBG_ERR); e = (_err)(e>>1), ++n)
				if(e & (1<<0))
					__print("\r\nerror %03d: %s",n, ErrMsg[n].c_str());	
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
	"pump stall",
	"pump pressure out of range",
	"pump current out of range",
	"fan 1 stall",
	"emergency button pressed",
	"handpiece ejected",
	"illegal status request",
	"energy report timeout",
	"spray not ready",
	"fan 2 stall"
};
