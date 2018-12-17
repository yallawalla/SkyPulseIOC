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
#include	"dl.h"
#include <algorithm>

_DL*	_DL::instance=NULL;
/*******************************************************************************/
/**
	* @brief
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
_DL::_DL() : high(300,150000), filter(5, 2000),filterRef(5, 2000) {
			instance=this;
			ton=toff=on=off=__time__;
			active=synced=false;
			timeout[0]=timeout[1]=offset[0]=offset[1]=0;
			HAL_ADC_Start_DMA(&hadc2, (uint32_t*)&dma, sizeof(dma)/sizeof(uint16_t));
			dlscale[0]=dlscale[0]=0;
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void	_DL::filterCbk(int k) {
unsigned short *p;
int		n=sizeof(dma)/sizeof(short)/4;
	
			if(k)
				p=&dma[n][0];
			else
				p=&dma[0][0];

			for(; n--; ++p,++p) {
				high.eval(p[0],p[1]);
			}

			if(__time__ < _DL_OFFSET_DELAY) {
				offset[0]=high.val[0];
				offset[1]=high.val[1];
			} else {
				filterRef.eval(ref[0],ref[1]);
				filter.eval(high.val[0]*(100-dlscale[0])/100-offset[0],high.val[1]*(100-dlscale[1])/100-offset[1]);
			}

			if(_TERM::debug & DBG_DL0)
				HAL_DAC_SetValue(&hdac,DAC_CHANNEL_2,DAC_ALIGN_12B_R, filter.val[0]);
			if(_TERM::debug & DBG_DL1)
				HAL_DAC_SetValue(&hdac,DAC_CHANNEL_2,DAC_ALIGN_12B_R, filterRef.val[0]);
			if(k && _TERM::debug & DBG_DL2)
				HAL_DAC_SetValue(&hdac,DAC_CHANNEL_2,DAC_ALIGN_12B_R, filter.val[0]);
			if(!k && _TERM::debug & DBG_DL2)
				HAL_DAC_SetValue(&hdac,DAC_CHANNEL_2,DAC_ALIGN_12B_R, filterRef.val[0]);
			if(k && _TERM::debug & DBG_DL3)
				HAL_DAC_SetValue(&hdac,DAC_CHANNEL_2,DAC_ALIGN_12B_R, filter.val[0]*50);
			if(!k && _TERM::debug & DBG_DL3)
				HAL_DAC_SetValue(&hdac,DAC_CHANNEL_2,DAC_ALIGN_12B_R, filterRef.val[0]*50);
			if(_TERM::debug & DBG_DL4)
				HAL_DAC_SetValue(&hdac,DAC_CHANNEL_2,DAC_ALIGN_12B_R, high.val[0]);
			if(_TERM::debug & DBG_DL5)
				HAL_DAC_SetValue(&hdac,DAC_CHANNEL_2,DAC_ALIGN_12B_R, high.val[0]*50);
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void	_DL::setTiming(int t1, int t2) {
			if(!t1 && !t2) {
				t1=498;
				t2=2;
			} else {
				t1 /=1000;
				t2 /=1000;
			}
			on=t1;
			off=t2;
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void	_DL::setLimits(int lim0, int lim1,int m) {
			lim[0]=lim0*5/6;
			lim[1]=lim1*5/6;
			mode=m;
			if(active && !synced) {
				ton=toff=__time__;
				synced=true;
			}
}
//______________________________________________________________________________________
//
//	DL levels check
//______________________________________________________________________________________
_err	_DL::Status(bool __active) {
_err 	e=_NOERR;
			if(__active != active) {
				active=__active;
				synced=false;
			}
			if(!synced) {
				ref[0]=ref[1]=0;
			} else if(on && off) {
				if(__time__ > ton) {
					ref[0]=lim[0];
					ref[1]=lim[1];
					toff=ton+on;
					ton=toff+off;
					if(high.val[0] < lim[0]/2+offset[0]) {
						++ton;
						++toff;
					} else {
						--ton;
						--toff;
					}
				} else if(__time__ > toff) {
					ref[0]=ref[1]=0;
					ton=toff+off;
					toff=ton+on;
				}
			}				

			if(filter.val[0] > filterRef.val[0] + std::max((int)lim[0]/5,_DL_OFFSET_THR))
					e = e | _DLpowerCh1;	
			if(filter.val[0] < filterRef.val[0] - std::max((int)lim[0]/5,_DL_OFFSET_THR))
				if(!timeout[0])
					timeout[0]=__time__ + _DL_ERROR_DELAY;
			if(filter.val[1] > filterRef.val[1] + std::max((int)lim[1]/5,_DL_OFFSET_THR))
					e = e | _DLpowerCh2;	
			if(filter.val[1] < filterRef.val[1] - std::max((int)lim[1]/5,_DL_OFFSET_THR))
				if(!timeout[1])
					timeout[1]=__time__ + _DL_ERROR_DELAY;
				
			
			if(active) {
				if(timeout[0] && __time__ > timeout[0]) {
					timeout[0]=0;
					if(mode & 1)
						e = e | _DLpowerCh1;
				}
				if(timeout[1] && __time__ > timeout[1]) {
					timeout[1]=0;
					if(mode & 2)
						e = e | _DLpowerCh2;
				}
			} else
				timeout[0]=timeout[1]=0;
			return e;
}
/*******************************************************************************/
/**
	* @brief
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
void	_DL::LoadSettings(FIL *f) {
char	c[128];
			f_gets(c,sizeof(c),f);
//			sscanf(c,"%d,%d,%d,%d,%d,%d,%d",&fpl,&fph,&ftl,&fth,&curr_limit,&flow_limit,&tacho_limit);
}
/*******************************************************************************/
/**
	* @brief
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
void	_DL::SaveSettings(FIL *f) {
//			f_printf(f,"%5d,%5d,%5d,%5d,%5d,%5d,%3d /.. pump\r\n",fpl,fph,ftl,fth,curr_limit,flow_limit,tacho_limit);
}
/*******************************************************************************/
/**
	* @brief
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
void 	_DL::Increment(int a, int b)	{
			idx= std::min(std::max(idx+b,0),3);
			switch(idx) {
				case 0:
					lim[0]= std::min(std::max((int)lim[0]+a,0),4095);
					break;
				case 1:
					lim[1]= std::min(std::max((int)lim[1]+a,0),4095);
					break;
				case 2:
					dlscale[0]= std::min(std::max(dlscale[0]-a,-100),100);
				case 3:
					dlscale[1]= std::min(std::max(dlscale[1]-a,-100),100);
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
int		_DL::Fkey(int t) {
			switch(t) {
				case __f4:
				case __F4:
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
				case __F1:
				case __f1:
				case __Delete:
					dlscale[0]=dlscale[1]=0;
					Increment(0,0);
				break;
				case __CtrlR:
				Increment(0,0);
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
void	_DL::Newline(void) {
//			_print("\r:pump  %3d%c,%2d.%d'C,%2d.%d",Rpm(100),'%',Th2o()/100,(Th2o()%100)/10,k/10,k%10);
			_print("\r:dl    %4d,%4d,%4d,%4d",lim[0],lim[1],std::max(0,(int)filter.val[0]),std::max(0,(int)filter.val[1]));
			for(int i=5*(3-idx)+1;i--;_print("\b")) {}
			Repeat(200,__CtrlR);
}

/**
* @}
*/ 


