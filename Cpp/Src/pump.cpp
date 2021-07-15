/**
  ******************************************************************************
  * @file 
  * @author 
  * @version  
  * @date 
  * @brief 
  *
  */
/** @addtogroup
* @{
*/
#include	"pump.h"
#include	"misc.h"
/*******************************************************************************/
/**
	* @brief
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
_PUMP::_PUMP() {
			ftl=34; fth=35; fpl=20; fph=50;
			mode=_PUMP_FLOW;
			curr_limit=33432;
			flow_limit=35;
			tacho_limit=8;
			idx=0;
			err=_NOERR;
			offset.cooler=gain.cooler=_BAR(1);
			timeout=__time__ + _PUMP_ERR_DELAY;
}
/*******************************************************************************/
/**
	* @brief
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
void	_PUMP::LoadSettings(FIL *f) {
char	c[128];
			f_gets(c,sizeof(c),f);
			sscanf(c,"%d,%d,%d,%d,%d,%d,%d",&fpl,&fph,&ftl,&fth,&curr_limit,&flow_limit,&tacho_limit);
}
/*******************************************************************************/
/**
	* @brief
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
void	_PUMP::SaveSettings(FIL *f) {
			f_printf(f,"%5d,%5d,%5d,%5d,%5d,%5d,%3d /.. pump\r\n",fpl,fph,ftl,fth,curr_limit,flow_limit,tacho_limit);
}
/*******************************************************************************/
/**
	* @brief
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
int		_PUMP::rpm(int fsc) {
			if(mode & _PUMP_BOOST)
				return fph*fsc/100;
			else
				return __ramp(th2o(),ftl*100,fth*100,fpl,fph)*fsc/100;

}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
void		_PUMP::Enable() {
				if(!pump_drive++) {
					err=_NOERR;
					timeout=__time__ +  _PUMP_ERR_DELAY;
				}
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
bool		_PUMP::Enabled() {
				return pump_drive;
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
void		_PUMP::Disable() {
				pump_drive=0;
}
/*******************************************************************************/
/**
	* @brief
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
_err	_PUMP::Status(void) {	
			if(pump_drive)
				pump_drive > rpm(1<<12) ? --pump_drive : ++pump_drive;
			if(__time__ > timeout) {
				err=_NOERR;
				if(tacho_limit && flow_limit && curr_limit) {
					if(pumpTacho-__pumpTacho <= tacho_limit)
						err = err | _pumpTacho;
					if(flowTacho-__flowTacho <= flow_limit)
						err = err | _flowTacho;						
					if(current() > curr_limit)
						err = err | _pumpCurrent;
				} else if(tacho_limit || flow_limit || curr_limit) {
					if(!pumpTacho)
						err = err | _pumpTacho;
					if(!flowTacho)
						err = err | _flowTacho;						
					if(!current())
						err = err | _pumpCurrent;
				}
				timeout=__time__+100;
				speed=pumpTacho-__pumpTacho;
				flow=flowTacho-__flowTacho;
				__pumpTacho=pumpTacho;
				__flowTacho=flowTacho;
				
				if(debug & DBG_INFO)
					HAL_DAC_SetValue(&hdac,DAC_CHANNEL_2,DAC_ALIGN_12B_R,speed*100);
				if(hw > _HW_INIT && floatLow)
					err = err | _floatError;
			}
			return err;
}
/*******************************************************************************/
/**
	* @brief
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
void 	_PUMP::Increment(int a, int b)	{
			idx= std::min(std::max(idx+b,0),5);
			switch(idx) {
				case 0:
					if(a)
						mode ^= _PUMP_FLOW;
					break;
				case 1:
					fpl= std::min(std::max(fpl+a,5),fph);
					break;
				case 2:
					fph= std::min(std::max(fph+a,fpl),95);
					break;
				case 3:
					ftl= std::min(std::max(ftl+a,0),fth);
					break;
				case 4:
					fth= std::min(std::max(fth+a,ftl),50);
					break;
			}
			Newline();
}
/*******************************************************************************/
/**
	* @brief
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
int		_PUMP::Fkey(int t) {
			switch(t) {
				case __f5:
				case __F5:
					return __F12;
				case __Up:
					Increment(1,0);
				break;
				case __Down:
					Increment(-1,0);
				break;
				case __Left:
					Increment(0,-1);
				break;
				case __Right:
					Increment(0,1);
				break;
				case __CtrlR:
				Increment(0,0);
				break;
				case __F1:
				case __f1:
				case __Delete:
				Setup();
				break;
			}
			return EOF;
}
/*******************************************************************************/
/**
	* @brief
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
void	_PUMP::Newline(void) {
int		k, i=current()*(3300/4096.0/0.05/21/16);
			if(mode & _PUMP_FLOW)
				k=flow/(2200/300);
			else
				k=10*(fval.cooler-offset.cooler)/gain.cooler;
			_print("\r:pump  %3d%c,%2d.%d'C,%2d.%d",rpm(100),'%',th2o()/100,abs(th2o()%100)/10,k/10,k%10);
			if(idx>0) {
				_print("   %2d%c-%2d%c,%2d'-%2d',%d.%03dA",fpl,'%',fph,'%',ftl,fth,i/1000,i%1000);
				for(int i=4*(6-idx)+2;idx && i--;_print("\b")) {}
			} else
				_print("\b");
			Repeat(200,__CtrlR);
}
/*******************************************************************************/
/**
	* @brief
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
bool	_PUMP::Setup(void) {
			if(tacho_limit || flow_limit || curr_limit) {
				_print("\r\npump errors disabled ...\r\n");
				tacho_limit=flow_limit=curr_limit=0;
			} else {
int 		i=fpl,j=fph;
				fph=fpl=i;
				for(int t=__time__ + 5000; __time__ < t;_wait(200))
					Newline();			
				tacho_limit=speed/2;
				flow_limit=flow/2;			
				fph=fpl=j;
				for(int t=__time__ + 5000; __time__ < t;_wait(200))
					Newline();		
				curr_limit=(current() * 11)/10;
				fpl=i;
				fph=j;
				_print("\r\npump errors enabled ...\r\n");
			}
			return true;
}
/**
* @}
*/ 


