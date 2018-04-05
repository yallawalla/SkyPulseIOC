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
			curr_limit=flow_limit=flow=0;
			Enabled=true;
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
			sscanf(c,"%d,%d,%d,%d,%d,%d",&fpl,&fph,&ftl,&fth,&curr_limit,&flow_limit);
}
/*******************************************************************************/
/**
	* @brief
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
void	_PUMP::SaveSettings(FIL *f) {
			f_printf(f,"%5d,%5d,%5d,%5d,%5d,%5d     /.. pump\r\n",fpl,fph,ftl,fth,curr_limit,flow_limit);
}
/*******************************************************************************/
/**
	* @brief
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
void	_PUMP::Newline(void) {
			_print("\r:pump      %5d%c,",Rpm(100),'%');
			_printdec(Th2o()/10,10);_print("'C, ");
			if(mode & (1<<PUMP_FLOW))
				_printdec(flow/2200,10);
			else
				_printdec(10*(fval.cooler-offset.cooler)/gain.cooler,10);
			if(idx>0) {
				int i=fval.Ipump*33/4096.0/2.1/16;
				_print("   %2d%c-%2d%c,%2d'C-%2d'C, ",fpl,'%',fph,'%',ftl,fth);
				_printdec(i,10);		
				_print(",%dmA",fval.Ipump/(int)(4096.0*2.1*16/3.3));
			}
			for(int i=4*(5-idx)+6;idx && i--;_print("\b"));
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
				if(Enabled==false) {
					timeout=__time__ +  _PUMP_ERR_DELAY;
					Enabled=true;
				}
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
void		_PUMP::Disable() {
				Enabled=false;
}
/*******************************************************************************/
/**
	* @brief
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
_err	_PUMP::Status(void) {	
_err	e=_NOERR;
			if(__time__ > _TACHO_ERR_DELAY) {
				pump_drive =Rpm(1<<12);
				if(__time__-pump_cbk > _PUMP_ERR_DELAY)
					e = e | _pumpTacho;	
			}
			return e;
}
/*******************************************************************************/
/**
	* @brief
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
void 	_PUMP::Increment(int a, int b)	{
			idx= std::min(std::max(idx+b,0),4);
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
/**
* @}
*/ 


