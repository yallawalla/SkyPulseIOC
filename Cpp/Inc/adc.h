#ifndef		_ADC_H
#define		_ADC_H

#include	"stm32f4xx_hal.h"
#include 	<stdlib.h>
#include 	"err.h"

/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
#define	_ADC_ERR_DELAY		200
#define	_UREF							3.3
#define	_Rdiv(a,b)				((a)/(a+b))
#define	_Rpar(a,b)				((a)*(b)/(a+b))
#define	_V5to16X					(int)(5.0/_UREF*_Rdiv(820.0,820.0)*65535.0+0.5)			
#define	_V12to16X					(int)(12.0/_UREF*_Rdiv(820.0,3300.0)*65535.0+0.5)			
#define	_V24to16X					(int)(24.0/_UREF*_Rdiv(820.0,6800.0)*65535.0+0.5)			
#define	_BAR(a)					 ((int)((a)*16384.0f))

			__inline 
int		__fit(int to, const int t[], const int ft[]) {
int			f3=(ft[3]*(t[0]-to)-ft[0]*(t[3]-to)) / (t[0]-t[3]);
int			f2=(ft[2]*(t[0]-to)-ft[0]*(t[2]-to)) / (t[0]-t[2]);
int			f1=(ft[1]*(t[0]-to)-ft[0]*(t[1]-to)) / (t[0]-t[1]);
				f3=(f3*(t[1]-to) - f1*(t[3]-to)) / (t[1]-t[3]);
				f2=(f2*(t[1]-to)-f1*(t[2]-to)) / (t[1]-t[2]);
				return(f3*(t[2]-to)-f2*(t[3]-to)) / (t[2]-t[3]);
}

const int Ttab[]={ 1000, 2500, 5000, 8000 };
const	int Rtab[]={ (0xffff*_Rdiv(18813.0,5100.0)), (0xffff*_Rdiv(10000.0,5100.0)), (0xffff*_Rdiv(3894.6,5100.0)), (0xffff*_Rdiv(1462.6,5100.0))};

typedef struct	{
			unsigned short	Ipump,T1,T2,V5,V12,V24,cooler,bottle,compressor,air;
		} adc;

class	_ADC {
	private:
	public:
	_ADC();
	_err		adcError(void);
	
	static	void adcFilter();
	static	adc val[], fval, gain, offset;
	static	int Th2o(void),Th2o(int);
};
#endif
