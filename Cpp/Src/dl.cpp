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
#include <math.h>
_DL*	_DL::instance=NULL;
/*******************************************************************************/
/**
	* @brief
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
_DL::_DL() : high(300,150000), filter(5, 1000) {
			instance=this;
			selected=emit=false;
			errtout[0]=errtout[1]=offset[0]=offset[1]=ton=toff=0;
			HAL_ADC_Start_DMA(&hadc2, (uint32_t*)&dma, sizeof(dma)/sizeof(uint16_t));
			dlscale[0]=dlscale[0]=0;
			scale=10;
			stest_delay=__time__ + _ADC_ERR_DELAY;
}
/*******************************************************************************
* Function Name	:
* Description		:
* Output				:
* Return				:
*******************************************************************************/
void	_DL::filterCbk(bool k) {

uint16_t *p,n=sizeof(dma)/sizeof(short)/4;
	
			k ? p=&dma[n][0] : p=&dma[0][0];
			while(n--) {
				high.eval(p[0]*(100-dlscale[0])/100,p[1]*(100-dlscale[1])/100);
				++p;++p;
			}

			if(stest_delay) {
				offset[0]=high.val[0];
				offset[1]=high.val[1];
			}

			if(_TERM::debug & DBG_INP0)
				HAL_DAC_SetValue(&hdac,DAC_CHANNEL_2,DAC_ALIGN_12B_R, scale*filter.val[0]);			//filter1
			if(_TERM::debug & DBG_INP1)
				HAL_DAC_SetValue(&hdac,DAC_CHANNEL_2,DAC_ALIGN_12B_R, scale*filter.val[1]);			//filter2
			if(_TERM::debug & DBG_REF0)
				HAL_DAC_SetValue(&hdac,DAC_CHANNEL_2,DAC_ALIGN_12B_R, scale*high.val[0]);				//high filter1
			if(_TERM::debug & DBG_REF1)
				HAL_DAC_SetValue(&hdac,DAC_CHANNEL_2,DAC_ALIGN_12B_R, scale*high.val[1]);				//high filter2
			
			if(k && _TERM::debug & DBG_DIFF0)
				HAL_DAC_SetValue(&hdac,DAC_CHANNEL_2,DAC_ALIGN_12B_R, scale*limits[0].val);			// razlika 0
			if(!k && _TERM::debug & DBG_DIFF0)
				HAL_DAC_SetValue(&hdac,DAC_CHANNEL_2,DAC_ALIGN_12B_R, scale*filter.val[0]);
			if(k && _TERM::debug & DBG_DIFF1)
				HAL_DAC_SetValue(&hdac,DAC_CHANNEL_2,DAC_ALIGN_12B_R, scale*limits[1].val);			// razlika 1
			if(!k && _TERM::debug & DBG_DIFF1)
				HAL_DAC_SetValue(&hdac,DAC_CHANNEL_2,DAC_ALIGN_12B_R, scale*filter.val[1]);
			if(k && _TERM::debug & DBG_DIFFab)
				HAL_DAC_SetValue(&hdac,DAC_CHANNEL_2,DAC_ALIGN_12B_R, scale*filter.val[0]);			// razlika hiva a,b
			if(!k && _TERM::debug & DBG_DIFFab)
				HAL_DAC_SetValue(&hdac,DAC_CHANNEL_2,DAC_ALIGN_12B_R, scale*filter.val[1]);
}
//______________________________________________________________________________________
//
//	DL levels check
//______________________________________________________________________________________
_err	_DL::Status(bool k) {
_err 	e=_NOERR;

			emit=k;
			if(selected && emit && ton && toff) {
				if (__time__ > ton) {
					if (toff <= ton) {
						toff = ton + limits[count].on;
					}
          switch (limits[count].mode) {
					 case 1:
						 filter.eval(limits[count].val - high.val[0] - offset[0],0);
					 break;
					 case 2:
						 filter.eval(0,limits[count].val - high.val[1] - offset[1]);
					 break;
					 default:
						 filter.eval(0,0);
					 break;
					}
				}
				if (__time__ > toff) {
					if(ton <= toff) {
						ton = toff + limits[count].off;
						high.val[limits[count].mode-1] < limits[count].val/2 ? --ton : ++ton;
						setActiveCh(++count);
					}
					filter.eval(high.val[0] - offset[0],high.val[1] - offset[1]);
				}
			} else {
				ton=toff=0;
				filter.eval(high.val[0] - offset[0],high.val[1] - offset[1]);
			}
/*
				limits *p=&limits[count];
				if(selected && emit) {
					if(ton && toff) {		
						if(__time__ >= ton) {																							// on sekvenca:
							toff=ton+p->on;																									// prenastavitev preklopnih casov
							ton=toff+p->off;
							if(high.val[p->mode-1] < p->val/2+offset[p->mode-1]) {
								++ton; ++toff;
							}	else {
								--ton; --toff;
							}
						} else if(__time__ >= toff) {
							ton=toff+p->off;																								// prenastavitev preklopnih casov
							toff=ton+p->on;
						}
					} else {
						filter.eval(high.val[0]*(100-dlscale[0])/100-offset[0],high.val[1]*(100-dlscale[1])/100-offset[1]);
						toff=ton=0;
					}
				}

//			if(fabs(filter.val[0] - filterRef.val[0]) > std::max((int)limits[0]/5,_DL_OFFSET_THR) && mode & 1)
//					e = e | _DLpowerCh1;	
//			if(fabs(filter.val[1] - filterRef.val[1]) > std::max((int)limits[1]/5,_DL_OFFSET_THR) && mode & 2)
//					e = e | _DLpowerCh2;

			if(filter.val[0] > filterRef.val[0] + std::max((int)limits[0]/5,_DL_OFFSET_THR))
					e = e | _DLpowerCh1;	
			if(filter.val[0] < filterRef.val[0] - std::max((int)limits[0]/5,_DL_OFFSET_THR))
				if(!errtout[0])
					errtout[0]=__time__ + _DL_ERROR_DELAY;
			if(filter.val[1] > filterRef.val[1] + std::max((int)limits[1]/5,_DL_OFFSET_THR))
					e = e | _DLpowerCh2;	
			if(filter.val[1] < filterRef.val[1] - std::max((int)limits[1]/5,_DL_OFFSET_THR))
				if(!errtout[1])
					errtout[1]=__time__ + _DL_ERROR_DELAY;

			if(emit) {
				if(errtout[0] && __time__ > errtout[0]) {
					errtout[0]=0;
					if(mode & 1)
						e = e | _DLpowerCh1;
				}
				if(errtout[1] && __time__ > errtout[1]) {
					errtout[1]=0;
					if(mode & 2)
						e = e | _DLpowerCh2;
				}
			} else
				errtout[0]=errtout[1]=0;
*/
//______________________________________________________________________________________
//
//	DL presence & selftest
//______________________________________________________________________________________

			if(stest_delay && __time__ > stest_delay) {
				switch(selftest()) {
					case 0x00:																// open
					case 0x0e:																// both low
						break;
					case 0x06:																// 1 low,		2 open																
					case 0x19:																// 1 high,	2 open
					case 0x1d:																// 1 high,	2 low
						e = e | _DLpowerCh1;
						break;
					case 0x0c:																// 1 open,	2 low
					case 0x13:																// 1 open, 	2 high
					case 0x17:																// 1 low, 	2 high
						e = e | _DLpowerCh2;
						break;
					default:
						e = e | _DLpowerCh1 | _DLpowerCh2;			// short,both high
						break;
				}
			}
			return e;
}	
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
uint8_t	_DL::setActiveCh(uint8_t n) {
			int i;
			count = n % ((sizeof limits)/sizeof(limit));
			for(i=0; !limits[count].mode && i<sizeof(limits)/sizeof(limit); ++i)
				count = ++count % (sizeof(limits)/sizeof(limit));
			if(i == sizeof(limits)/sizeof(limit))
				count=0;
			return count;
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void	_DL::Setup() {		
			selected=false;
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void	_DL::Setup(DL_Limits *p) {
			selected=true;
			if(emit && !ton && !toff) {
				count=setActiveCh(0);
				ton=toff=__time__;
			}	else {
				limits[0].val	= (p->l0 * 5)/6;
				limits[1].val	= (p->l1 * 5)/6;
				limits[2].val	= (p->l2 * 5)/6;
				limits[0].mode	= p->ch0;
				limits[1].mode	= p->ch1;
				limits[2].mode	= p->ch2;
			}				
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void	_DL::Setup(DL_Timing *p) {		
			limits[p->ch-1].on	= p->on/1000;
			limits[p->ch-1].off	= p->off/1000;
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
}
/*******************************************************************************/
/**
	* @brief
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
void	_DL::SaveSettings(FIL *f) {
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
					limits[0].val= std::min(std::max((int)limits[0].val+a,0),4095);
					break;
				case 1:
					limits[1].val= std::min(std::max((int)limits[1].val+a,0),4095);
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
					selftest();
					_print("\r\n\r\n%4d,%4d,%4d,%2d\r\n%4d,%4d,%4d,%2d\r\n%4d,%4d,%4d,%2d\r\n\r\n",
						limits[0].val,limits[0].on,limits[0].off,limits[0].mode,
						limits[1].val,limits[1].on,limits[1].off,limits[1].mode,
						limits[2].val,limits[2].on,limits[2].off,limits[2].mode);
				break;
				case __Delete:
					dlscale[0]=dlscale[1]=0;
					Increment(0,0);
				break;
				case __CtrlR:
				Increment(0,0);
				break;
				case __PageUp:
				scale=std::min(scale+1,100);
				break;
				case __PageDown:
				scale=std::max(scale-1,1);
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
			switch(stest_err) {
				case 0:
					_print("\r:dl ---                             ");
				break;
				case 0x0e:
					_print("\r:dl    %4d,%4d,%4d,%4d,%4d,%4d",limits[0].val,limits[1].val,std::max(0,(int)filter.val[0]),std::max(0,(int)filter.val[1]),(int)offset[0],(int)offset[1]);
				break;
				default:
					_print("\r:dl err(%02X)                         ",stest_err);
				break;
			}
			for(int i=5*(3-idx)+11;i--;_print("\b"));
			Repeat(200,__CtrlR);
}
/*******************************************************************************/
/**
	* @brief
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
int	_DL::selftest(void) {
	int	n=stest_err=0;
	stest_delay=__time__ + 200;
	do {
		switch(n) {
			case 0:
				if(high.val[0] > 0xfff/10 || high.val[1] > 0xfff/10)
					stest_err |= (1<<n);
				GPIOB->PUPDR = (GPIOB->PUPDR & 0x0fffffff) | 0x90000000;
				break;
			case 1:
				if(high.val[0] < 9*0xfff/10 || high.val[1] > 0xfff/10)
					stest_err |= (1<<n);
				GPIOB->PUPDR = (GPIOB->PUPDR & 0x0fffffff) | 0x50000000;
				break;
			case 2:
				if(high.val[0] < 9*0xfff/10 || high.val[1] < 9*0xfff/10)
					stest_err |= (1<<n);
				GPIOB->PUPDR = (GPIOB->PUPDR & 0x0fffffff) | 0x60000000;
				break;
			case 3:
				if(high.val[0] > 0xfff/10 || high.val[1] < 9*0xfff/10)
					stest_err |= (1<<n);
				GPIOB->PUPDR = (GPIOB->PUPDR & 0x0fffffff) | 0xA0000000;
				break;
			case 4:
				if(high.val[0] > 0xfff/10 || high.val[1] > 0xfff/10)
					stest_err |= (1<<n);	
				break;
			default:
				break;
		}
		_wait(20);
	} while(n++ < 4);
	_wait(200);
	stest_delay=0;
	return stest_err;
}
/**
* @}
*/ 


