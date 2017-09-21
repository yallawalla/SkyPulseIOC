#include "stm32f4xx_hal.h"
#include "ioc.h"

extern "C" {

void ioc(void) {
	_IOC ioc;
	while(true)
		_proc_loop();
	}
}
_Error 	_IOC::error_mask	=_NOERR;
_DEBUG_	_IOC::debug				= DBG_OFF;
/*******************************************************************************
* Function Name	:
* Description		:
* Output				:
* Return				:
*******************************************************************************/
_IOC::_IOC() {
	SetState(_STANDBY);
	com=new 	_FS();
	com1=new 	_FS(&huart1);
	com3=new 	_FS(&huart3);
	
	can=_CAN::InstanceOf(&hcan2);
	can->canFilterCfg(idIOC_State,	0x780);
	can->canFilterCfg(idEC20_req,		0x780);
	can->canFilterCfg(idEM_ack,			0x7ff);
	can->canFilterCfg(idBOOT,				0x7ff);

	error_mask = _NOERR;
	_proc_add((void *)poll,this,(char *)"can task",0);
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
void	*_IOC::poll(void *v) {
			_IOC *me=static_cast<_IOC *>(v);
			me->can->Task(me);
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
							ErrParse(_illstatereq);
						break;
					case	_ACTIVE:
						if(IOC_State.State == _READY) {
							IOC_State.State = _ACTIVE;
							//Submit("@active.led");
						} else
							ErrParse(_illstatereq);
						break;
					case	_ERROR:
						IOC_State.State = _ERROR;
						//Submit("@error.led");
						_SYS_SHG_DISABLE;
						break;
					default:
						ErrParse(_illstatereq);
						break;
				}
}
/*******************************************************************************
* Function Name	:
* Description		:
* Output				:
* Return				:
*******************************************************************************/
void	_IOC::ErrParse(_Error e) {

			e = (_Error)(e & ~error_mask);
			e ? led.RED1(3000): led.GREEN1(20);
			e = (_Error)((e ^ IOC_State.Error) & e);
	
			IOC_State.Error = (_Error)(IOC_State.Error | e);

			if(e) {
				_SYS_SHG_DISABLE;
				IOC_State.State = _ERROR;
				IOC_State.Send();
//				if(IOC_State.State != _ERROR)
					//Submit("@error.led");
			}

			for(int n=0; e && _IOC::debug & (1<<DBG_ERR); e = (_Error)(e>>1), ++n)
				if(e & (1<<0))
					printf("\r\nerror %03d: %s",n, ErrMsg[n].c_str());	
}
/*******************************************************************************
* Function Name	:
* Description		:
* Output				:
* Return				:
*******************************************************************************/
string _IOC::ErrMsg[] = {
	"5V  supply",
	"12V supply",
	"24V supply",
	"spray input pressure",
	"cooler temperature",
	"pump speed out of range",
	"pump pressure out of range",
	"pump current out of range",
	"fan speed out of range",
	"emergency button pressed",
	"handpiece ejected",
	"illegal status request",
	"energy report timeout"
};
