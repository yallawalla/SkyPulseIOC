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
	error_mask = _fan2Tacho | _sprayInPressure | _sprayNotReady;
	
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
			me->adcSmooth();
			me->SetError(me->pump.Status());
			me->SetError(me->fan.Status());
			me->SetError(me->spray.Status());
			me->SetError(me->adcError());
			
			me->led.poll();
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
	
			if(ee && HAL_GetTick() > 3000) {
				_SYS_SHG_DISABLE;
//				if(IOC_State.State != _ERROR)
//					Submit("@error.led");
				IOC_State.Error = (_err)(IOC_State.Error | ee);
				IOC_State.State = _ERROR;
				IOC_State.Send();
			} 

//int		ww=(e ^ IOC_State.Error) & warn_mask;
//			if(ww && HAL_GetTick() > 3000) {
//				IOC_State.Error = (_Error)(IOC_State.Error ^ ww);
//				IOC_State.Send();
//			} 
			
			if(_SYS_SHG_ENABLED)
				led.GREEN1(200);
			else
				led.RED1(200);

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
//void	_IOC::SetError(_err e) {

//			e = e & ~error_mask;
//			e ? led.RED1(300): led.GREEN1(300);
//			e = (e ^ IOC_State.Error) & e;
//	
//			IOC_State.Error = (_err)(IOC_State.Error | e);

//			if(e) {
//				_SYS_SHG_DISABLE;
//				IOC_State.State = _ERROR;
//				IOC_State.Send();
////				if(IOC_State.State != _ERROR)
//					//Submit("@error.led");
//			}

//			for(int n=0; e && debug & (1<<DBG_ERR); e = (_err)(e>>1), ++n)
//				if(e & (1<<0))
//					_print("\r\nerror %03d: %s",n, ErrMsg[n].c_str());	
//}
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
