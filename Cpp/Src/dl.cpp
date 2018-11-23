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
_DL*	_DL::instance=NULL;
/*******************************************************************************/
/**
	* @brief
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
_DL::_DL() : filter(5, SystemCoreClock/2/4/(12+56)/2),filterRef(5, SystemCoreClock/2/4/(12+56)/2) {
		instance=this;
		ton=toff=on=off=0;
		active=false;
		HAL_ADC_Start_DMA(&hadc2, (uint32_t*)&dma, sizeof(dma)/sizeof(uint16_t));
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
			
			while(n--) {
				filterRef.eval(ref[0],ref[1]);
				filter.eval(p[0],p[1]);
				++p;++p;
			}
			if(__time__ < _DL_OFFSET_DELAY) {
				offset[0]=filter.Val[0];
				offset[1]=filter.Val[1];
			}
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void	_DL::setLimits(int lim0, int lim1) {
			filterRef.reset();
			ton=toff=0;
			max[0]=lim0;
			max[1]=lim1;
}	
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void	_DL::setTiming(int t1, int t2) {
			filterRef.reset();
			ton=toff=0;
			if(!t1 && !t2) {
				t1=495;
				t2=5;
			} else {
				t1 /=1000;
				t2 /=1000;
			}
			if(t1 + t2 > 500) {
				t1=500*t1/(t1 + t2);
				t2=500*t2/(t1 + t2);
			}
			on=t1;
			off=t2;
}	
//______________________________________________________________________________________
//
//	DL levels check
//______________________________________________________________________________________
_err	_DL::Status(bool __active) {
_err 	e=_NOERR;
			if(__active != active) {
				active=__active;
				filter.reset();
			}
			if(on && off) {
				if(__time__ > ton) {
					ref[0]=max[0]+offset[0];
					ref[1]=max[1]+offset[1];
					toff=__time__+on;
					ton=toff+off;
					filterRef.Min[0]=filterRef.Val[0];
					filterRef.Min[1]=filterRef.Val[1];
				} else if(__time__ > toff) {
					ref[0]=offset[0];
					ref[1]=offset[1];
					ton=__time__+off;
					toff=ton+on;
					filterRef.Max[0]=filterRef.Val[0];
					filterRef.Max[1]=filterRef.Val[1];		
				}
			}				

			if(active) {
				if(filter.Val[0] > filterRef.Max[0] + _DL_OFFSET_THR)
					e = e | _DLpowerCh1;	
				if(filter.Val[1] > filterRef.Max[1] + _DL_OFFSET_THR)
					e = e | _DLpowerCh2;	
			} else {
				if(filter.Val[0] > offset[0] + _DL_OFFSET_THR)
					e = e | _DLpowerCh1;	
				if(filter.Val[1] > offset[1] + _DL_OFFSET_THR)
					e = e | _DLpowerCh2;	
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
int		_DL::Fkey(int t) {
			switch(t) {
				case __f5:
				case __F5:
					return __F12;
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

}
/**
* @}
*/ 


