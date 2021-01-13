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
_DL::_DL() : high(300,150000), filter(2.5, 1000), max(2.5, 1000) {
			instance=this;
			selected=false;
			offset[0]=offset[1]=ton=toff=0;
			HAL_ADC_Start_DMA(&hadc2, (uint32_t*)&dma, sizeof(dma)/sizeof(uint16_t));
			dlscale[0]=dlscale[0]=0;
			dacScale=1;
			dacOffset=1000;
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

			if(stest_delay) {
				while(n--) {
					high.eval(p[0],p[1]);
					++p;++p;
				}
				offset[0]=high.val[0];
				offset[1]=high.val[1];
			} else
				while(n--) {
					high.eval((p[0]*(100-dlscale[0]))/100- offset[0],(p[1]*(100-dlscale[1]))/100- offset[1]);
					++p;++p;
				}
			
			dac(10,high.val[0]);
			dac(11,high.val[1]);
			dac(12,ref[0]);
			dac(13,ref[1]);
			dac(14,filter.val[0]);
			dac(15,filter.val[1]);

			if(k) dac(16,high.val[0]);		else dac(16,ref[0]);
			if(k) dac(17,high.val[1]);		else dac(17,ref[1]);
			if(k) dac(18,filter.val[0]);	else dac(18,max.val[0]);
			if(k) dac(19,filter.val[1]);	else dac(19,max.val[1]);
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
_err	_DL::Check(float ch1, float ch2) {
_err	e=_NOERR;
			if(abs((int)(__time__ - ton)) > 2 && abs((int)(__time__ - toff)) > 2) {					// skip on transients
				filter.eval(ch1 - ref[0], ch2 - ref[1]);																			// ref vs. high/default difference
				max.eval(ref[0],ref[1]);																											// ref. filter.
			}
			if(fabs(filter.val[0]) > std::max(_DL_OFFSET_THR,(int)max.val[0]/5))						// error triggers
				e = e | _DLpowerCh1;
			if(fabs(filter.val[1]) > std::max(_DL_OFFSET_THR,(int)max.val[1]/5))
				e = e | _DLpowerCh2;
			return e;
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
_err	_DL::Status(bool k) {
_err 	e=_NOERR;
			if(!selected || (limits[0].mode==0 && limits[1].mode==0 && limits[2].mode==0))	// deny active if not selected or no active channels
				k=false;
			ref[0]=ref[1]=0;
			if(k) {																																					// check on active			
				if(ton==0 && toff==0) {																												// first active entry
					setActiveCh(0);
					ton=toff=__time__;
				}
				if (__time__ >= ton) {																												// leading edge
					if (toff <= ton)																														// flip off time															
						toff = ton + limits[active].on;
					switch (limits[active].mode) {																							// process active channel
						case 1:
							ref[0]=limits[active].val;
							e = e | Check(high.val[0], 0);
						break;
						case 2:
							ref[1]=limits[active].val;
							e = e | Check(0, high.val[1]);
						break;
						default:
							e = e | Check(high.val[0], high.val[1]);																// shouldn't be here !!!
					 break;
					}
				}
				if (limits[active].on && limits[active].off && __time__ > toff) {							// trailing edge, deny on CW
					if(ton <= toff) {																														// flip on time	
						ton = toff + limits[active].off;
						high.val[limits[active].mode-1] < limits[active].val/2 ? --ton : ++ton;		// sync trailing edge on active
						setActiveCh(++active);
					}
					e = e | Check(high.val[0], high.val[1]);
				}
			} else {																																				// processing standby, ready or fsw break;
				ton=toff=limits[0].mode=limits[1].mode=limits[2].mode=0;
				e = e | Check(high.val[0], high.val[1]);
			}
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
void	_DL::setActiveCh(uint32_t n) {
			int i;
			active = n % 3;
			for(i=0; !limits[active].mode && i<3; ++i)
				active = ++active % 3;
			if(i == 3)
				active=0;
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
			limits[0].val	= 	p->l0;
			limits[1].val	= 	p->l1;
			limits[2].val	= 	p->l2;
			limits[0].mode	= p->ch0;
			limits[1].mode	= p->ch1;
			limits[2].mode	= p->ch2;	
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
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void	_DL::LoadSettings(FIL *f) {
char	c[128];
			f_gets(c,sizeof(c),f);
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void	_DL::SaveSettings(FIL *f) {
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void 	_DL::Increment(int a, int b)	{
			idx= std::min(std::max(idx+b,0),3);
			switch(idx) {
				case 0:
					dlscale[0]= std::min(std::max(dlscale[0]-a,-100),100);
					break;
				case 1:
					dlscale[1]= std::min(std::max(dlscale[1]-a,-100),100);
					break;
				case 2:
					break;
				case 3:
					break;
			}
			Newline();
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
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
					if(!ton && !toff)
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
				dacScale=std::min(dacScale+1,100);
				break;
				case __PageDown:
				dacScale=std::max(dacScale-1,1);
				break;
				case __Home:
				dacOffset=std::min(dacOffset+100,3000);
				break;
				case __End:
				dacOffset=std::max(dacOffset-100,0);
				break;
			}
			return EOF;
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void	_DL::Newline(void) {
			switch(stest_err) {
				case 0:
					_print("\r:dl ---                             ");
				break;
				case 0x0e:
					_print("\r:dl    %4d,%4d,%4d,%4d,%4d,%4d",
						(int)(max.val[0] * (100-dlscale[0]))/100,
						(int)(max.val[1] * (100-dlscale[1]))/100,
						(int)filter.val[0],(int)filter.val[1],
						(int)offset[0],(int)offset[1]);
				break;
				default:
					_print("\r:dl err(%02X)                         ",stest_err);
				break;
			}
			for(int i=5*(3-idx)+11;i--;_print("\b"));
			Repeat(200,__CtrlR);
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
int		_DL::selftest(void) {
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
/*
tandem
____---____________---____________---____________---____________---_________
___________-----__________-----__________-----__________-----__________-----___
*/


