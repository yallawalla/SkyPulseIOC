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
#define	_PI								3.141592653589793

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

struct lopass {
	private:
		struct _lopass {
			private:
				float x,dx,k;
			public:
				_lopass(float fo, float fs) {
					x=dx=0;
					k=2*_PI*fo/fs;
				}
				float eval(float inp) {
					float _dx = inp-x-dx-dx;
					x += k*dx;
					dx += k*_dx;
					return x;
				}
				void reset() {
					x=dx=0;
				}
				float max,min;
		} ch1,ch2;
	public:
		float X[2];
		lopass(float fo, float fs) : ch1(fo,fs),ch2(fo,fs) {}
		void eval(float in0,float in1) {
			X[0]=ch1.eval(in0);
			X[1]=ch2.eval(in1);
		}	
		void reset() {
			ch1.reset();
			ch2.reset();
		}	
};


typedef struct _diode {
			float offset[2],
						max[2],
						min[2],
						ref[2];
			unsigned short	dma[154][2];
			unsigned int 		ton,toff;
			lopass	filterHi, filterLow;
			_diode():filterHi(150, SystemCoreClock/2/4/(12+56)/2),filterLow(5, 1000) {}
		} diode;

		class	_ADC {
	private:
	public:
	_ADC();
	_err		adcError(void);
	
	static	void adcFilter(),diodeFilter(int);
	static	adc val[], fval, gain, offset;
	static	diode DL;
	static	int Th2o(void),Th2o(int);
	
};
#endif
