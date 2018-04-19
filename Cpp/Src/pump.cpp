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
_PUMP::_PUMP()  {
			ftl=25; fth=40; fpl=10; fph=50;
			offset.cooler=_BAR(1);
			gain.cooler=_BAR(1);
			idx=0;
			timeout=__time__ + _PUMP_ERR_DELAY;
			mode=(1<<PUMP_FLOW);
			tacho_limit=curr_limit=flow_limit=flow=speed=0;
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
int		_PUMP::Rpm(int fsc) {
			return __ramp(Th2o(),ftl*100,fth*100,fpl,fph)*fsc/100;
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
void		_PUMP::Enable() {
				if(!speed++)
					timeout=__time__ +  _PUMP_ERR_DELAY;
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
void		_PUMP::Disable() {
				speed=0;
}
/*******************************************************************************/
/**
	* @brief
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
_err	_PUMP::Status(void) {	
int		e=_NOERR;
			if(speed) {
				if(pump_drive < Rpm(1<<12))
					++pump_drive;
				else
					--pump_drive;
			} else
				pump_drive=0;
			
			if(__time__ > timeout) {
				if(tacho_limit && (pumpTacho-__pumpTacho) <= tacho_limit)
					e |= _pumpTacho;
				if(curr_limit && fval.Ipump > curr_limit)
					e |= _pumpCurrent;
				if(flow_limit && (flowTacho-__flowTacho) <= flow_limit)
					e |= _flowTacho;				
				timeout=__time__+100;
				
				if(speed)
					speed=pumpTacho-__pumpTacho;
				flow=flowTacho-__flowTacho;
				__pumpTacho=pumpTacho;
				__flowTacho=flowTacho;
			} 	
			return (_err)e;
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
void	_PUMP::Newline(void) {
int		k, i=fval.Ipump*3300/4096.0/2.1/16;
			if(mode & (1<<PUMP_FLOW))
				k=flow/(2200/600);
			else
				k=(fval.cooler-offset.cooler)/gain.cooler;
			_print("\r:pump  %3d%c,%2d.%d'C,%2d.%d",Rpm(100),'%',Th2o()/100,(Th2o()%100)/10,k/10,k%10);
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
			if(tacho_limit) {
				_print("\r\npump errors disabled ...\r\n");
				tacho_limit=flow_limit=curr_limit=0;
			} else {
int 		i=fph;
				fph=fpl;
				_wait(2500);
				tacho_limit=speed/2;
				flow_limit=flow/2;
				fph=i;
				i=fpl;
				fpl=fph;
				_wait(2500);
				curr_limit=(fval.Ipump * 6)/5;
				fpl=i;
				_print("\r\npump errors enabled ...\r\n");
			}
			return true;
}
/**
* @}
*/ 


