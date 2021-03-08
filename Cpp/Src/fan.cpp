/**
  ******************************************************************************
  * @file    dac.cpp
  * @author  Fotona d.d.
  * @version V1
  * @date    30-Sept-2013
  * @brief	 DA & DMA converters initialization
  *
  */
/** @addtogroup
* @{
*/
#include	"fan.h"
#include	"misc.h"
/*******************************************************************************/
/**
	* @brief
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
_FAN::_FAN()  {
			ftl=30; fth=40; fpl=20; fph=95;
			tacho_limit=4;
			timeout=__time__ + _FAN_ERR_DELAY;
			idx=mode=0;
			err=_NOERR;
}
/*******************************************************************************/
/**
	* @brief
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
void	_FAN::LoadSettings(FIL *f) {
char	c[128];
			f_gets(c,sizeof(c),f);
			sscanf(c,"%d,%d,%d,%d,%d",&fpl,&fph,&ftl,&fth,&tacho_limit);
}
/*******************************************************************************/
/**
	* @brief
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
void	_FAN::SaveSettings(FIL *f) {
			f_printf(f,"%5d,%5d,%5d,%5d,%5d           /.. fan\r\n",fpl,fph,ftl,fth,tacho_limit);
}
/*******************************************************************************/
/**
	* @brief
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
int		_FAN::Fkey(int t) {
			switch(t) {
					case __f6:
					case __F6:
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
int		_FAN::rpm(int fsc) {
			int plow;
			switch(mode) {
				case 0:
					plow=fpl;
					break;
				case _FAN_BOOST0:
					plow=(fph-fpl)/3 + fpl;
					break;
				case _FAN_BOOST1:
					plow=((fph-fpl)*2)/3 + fpl;
					break;
				case _FAN_BOOST2:
					plow=((fph-fpl)*13)/15 + fpl;
					break;
				default:
					plow=fph;
				}
			return __ramp(th2o(),ftl*100,fth*100,plow,fph)*fsc/100;
}
/*******************************************************************************/
/**
	* @brief
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
_err	_FAN::Status(void) {
			if(__time__ % 4 == 0)
				fan_drive > rpm(__PWMRATE) ? --fan_drive : ++fan_drive;
			if(__time__ > timeout) {
				err=_NOERR;
				if(tacho_limit && (fanTacho-__fanTacho) <= tacho_limit)
					err = err | _fanTacho;
				timeout=__time__+100;
				speed=fanTacho-__fanTacho;
				__fanTacho=fanTacho;
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
void 	_FAN::Increment(int a, int b)	{
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
/*******************************************************************************/
/**
	* @brief
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
void	_FAN::Newline(void) {
			int dt=th2o(0)-th2o(1);
			_print("\r:fan   %3d%c,%2d.%d'C",rpm(100),'%',th2o()/100,abs(th2o()%100)/10);
			if(idx>0) {
				_print("        %2d%c-%2d%c,%2d'-%2d'",fpl,'%',fph,'%',ftl,fth);      
				dt < 0 ? _print(",-") : _print(", ");
				_print("%d.%02d'C",abs(dt/100),abs(dt%100));
			}
			for(int i=4*(6-idx)+3;idx && i--;_print("\b"));
			Repeat(200,__CtrlR);
}
/*******************************************************************************/
/**
	* @brief
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
bool	_FAN::Setup(void) {
			if(tacho_limit) {
				_print("\r\nfan errors disabled ...\r\n");
				tacho_limit=0;
			} else {
int 		i=fpl,j=fph;
				fph=fpl=i;
				for(int t=__time__ + 5000; __time__ < t;_wait(200))
					Newline();		
				tacho_limit=speed/2;
				fpl=i;
				fph=j;
				_print("\r\nfan errors enabled ...\r\n");
			}
			return true;
}/**
* @}
*/ 


