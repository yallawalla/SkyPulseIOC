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
			selected=emit=false;
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
			if(k) dac(18,filter.val[0]);	else dac(18,ref[0]);
			if(k) dac(19,filter.val[1]);	else dac(19,ref[1]);
			if(k) dac(20,max.val[0]);			else dac(20,max.val[1]);
}
//______________________________________________________________________________________
//
//	DL levels check
//______________________________________________________________________________________
_err	_DL::Check(float ch1, float ch2) {
_err	e=_NOERR;
			if(abs((int)(__time__ - ton)) > 2 && abs((int)(__time__ - toff)) > 2) {
				filter.eval(ch1 - ref[0], ch2 - ref[1]);
				max.eval(std::max(_DL_OFFSET_THR,(int)ref[0]/5), std::max(_DL_OFFSET_THR,(int)ref[1]/5));
			}
			if(fabs(filter.val[0])  > max.val[0])
				e = e | _DLpowerCh1;
			if(fabs(filter.val[1])  > max.val[1])
				e = e | _DLpowerCh2;
			return e;
}
//______________________________________________________________________________________
//
//	DL levels check
//______________________________________________________________________________________
_err	_DL::Status(bool k) {
_err 	e=_NOERR;

			emit=k;
			ref[0]=ref[1]=0;
			if(selected && emit && ton && toff) {
				if (__time__ > ton) {
					if (toff <= ton)
						toff = ton + limits[count].on;
					switch (limits[count].mode) {
						case 1:
							ref[0]=limits[count].val;
							e = e | Check(high.val[0], 0);
						break;
						case 2:
							ref[1]=limits[count].val;
							e = e | Check(0, high.val[1]);
						break;
						default:
							e = e | Check(high.val[0], high.val[1]);
					 break;
					}
				}
				if (limits[count].off && __time__ > toff) {
					if(ton <= toff) {
						ton = toff + limits[count].off;
						high.val[limits[count].mode-1] < limits[count].val/2 ? --ton : ++ton;
						setActiveCh(++count);
					}
					e = e | Check(high.val[0], high.val[1]);
				}
			} else {
				ton=toff=0;
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
uint8_t	_DL::setActiveCh(uint8_t n) {
			int i;
			count = n % 3;
			for(i=0; !limits[count].mode && i<3; ++i)
				count = ++count % 3;
			if(i == 3)
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
				setActiveCh(0);
				ton=toff=__time__;
			}	else {
				if( p->ch0 || p->ch1 || p->ch2) {
					limits[0].val	= (p->l0 * 5)/6;
					limits[1].val	= (p->l1 * 5)/6;
					limits[2].val	= (p->l2 * 5)/6;
					limits[0].mode	= p->ch0;
					limits[1].mode	= p->ch1;
					limits[2].mode	= p->ch2;
				}
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
					_print("\r:dl    %4d,%4d,%4d,%4d,%4d,%4d",
						(limits[0].val * (100-dlscale[0]))/100,(limits[1].val*(100-dlscale[1]))/100,
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
/*
tandem
____---____________---____________---____________---____________---_________
___________-----__________-----__________-----__________-----__________-----___
*/


