#ifndef					ADC_H
#define					ADC_H

typedef struct	{
unsigned short	T2,T3,V5,V12,V24,cooler,bottle,compressor,air,Ipump;
} _ADMA;

typedef struct	{
unsigned short	D1,D2;
} _DIODE;


__inline 
int			__fit(int to, const int t[], const int ft[]) {
int			f3=(ft[3]*(t[0]-to)-ft[0]*(t[3]-to)) / (t[0]-t[3]);
int			f2=(ft[2]*(t[0]-to)-ft[0]*(t[2]-to)) / (t[0]-t[2]);
int			f1=(ft[1]*(t[0]-to)-ft[0]*(t[1]-to)) / (t[0]-t[1]);
				f3=(f3*(t[1]-to) - f1*(t[3]-to)) / (t[1]-t[3]);
				f2=(f2*(t[1]-to)-f1*(t[2]-to)) / (t[1]-t[2]);
				return(f3*(t[2]-to)-f2*(t[3]-to)) / (t[2]-t[3]);
}

#define	_12V_ENABLE				GPIO_ResetBits(_12Voff_PORT,_12Voff_PIN)
#define	_12Voff_DISABLE		GPIO_SetBits(_12Voff_PORT,_12Voff_PIN)

#define	_BAR(a)						((float)(a)*16020.0f)		// see MPXH6400A specs...
#define	_UREF							3.3f
#define	_Rdiv(a,b)				((a)/(a+b))
#define	_Rpar(a,b)				((a)*(b)/(a+b))
#define	_V5to16X					(int)(5.0f/_UREF*_Rdiv(820.0f,820.0f)*65535.0f+0.5f)			
#define	_V12to16X					(int)(12.0f/_UREF*_Rdiv(820.0f,3300.0f)*65535.0f+0.5f)			
#define	_V24to16X					(int)(24.0f/_UREF*_Rdiv(820.0f,6800.0f)*65535.0f+0.5f)			
	
#define	_16XtoV5(a)				(float)((float)a/65535.0f*_UREF/_Rdiv(820.0f,820.0f))			
#define	_16XtoV12(a)			(float)((float)a/65535.0f*_UREF/_Rdiv(820.0f,3300.0f))			
#define	_16XtoV24(a)			(float)((float)a/65535.0f*_UREF/_Rdiv(820.0f,6800.0f))			

const int Ttab[]={ 1000, 2500, 5000, 8000 };
const	int Rtab[]={ (0xffff*_Rdiv(18813.0,5100.0)), (0xffff*_Rdiv(10000.0,5100.0)), (0xffff*_Rdiv(3894.6,5100.0)), (0xffff*_Rdiv(1462.6,5100.0))};

class	_ADC {
	private:
		int			n,timeout;
		static _ADC *instance;
	public:
		_ADC();
		static _ADMA	adc[],adf,offset,gain;
		static _DIODE	diode[];
		static void		Initialize_ADC1();
		static void		Initialize_ADC2();
		static int		Status(void);
		static int		Th2o(void);
		static void		adcFilter(void);
		static void		diodeFilter(int );
	};
#endif
