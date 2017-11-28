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
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
_PUMP::_PUMP()  {
			ftl=25; fth=40; fpl=10; fph=50;
			offset.cooler=12500;
			gain.cooler=13300;
			idx=0;
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
void	_PUMP::LoadSettings(FIL *f) {
char	c[128];
			f_gets(c,sizeof(c),f);
			sscanf(c,"%d,%d,%d,%d",&fpl,&fph,&ftl,&fth);
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
void	_PUMP::SaveSettings(FIL *f) {
			f_printf(f,"%5d,%5d,%5d,%5d                 /.. pump\r\n",fpl,fph,ftl,fth);
}
//_________________________________________________________________________________
void	_PUMP::Newline(void) {
				_print("\r:pump      %5d%c,%4.1lf'C,%4.1lf",Rpm(100),'%',(double)Th2o()/100,(double)(fval.cooler-offset.cooler)/gain.cooler);
				if(idx>0)
					_print("   %2d%c-%2d%c,%2d'C-%2d'C,%4.3lf",fpl,'%',fph,'%',ftl,fth,(double)fval.Ipump/4096.0*3.3/2.1/16);		
				for(int i=4*(5-idx)+6;idx && i--;_print("\b"));
}
//_________________________________________________________________________________
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
int		_PUMP::Rpm(int fsc) {
			return __ramp(Th2o(),ftl*100,fth*100,fpl,fph)*fsc/100;
}
/*******************************************************************************/
_err _PUMP::Status(void) {	
			pump_drive =Rpm(1<<12);
			if(HAL_GetTick() > _TACHO_ERR_DELAY) {
				if(HAL_GetTick()-pump_cbk > _PUMP_ERR_DELAY)
					return _pumpTacho;	
			}
			return _NOERR;
}
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

