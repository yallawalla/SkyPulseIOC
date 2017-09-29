#ifndef					SPRAY_H
#define					SPRAY_H

#include	<stdio.h>
#include	"term.h"
#include	"adc.h"
#include	"err.h"
#include	"ff.h"
#include	<algorithm>

#define					_BAR(a) ((a)*16384.0)
extern void			Simulate(void);					

typedef	struct {
	bool	On:1;
	bool	Vibrate:1;
	bool	Simulator:1;
	bool	Ready:1;
}	mode;

class	_SPRAY:public _ADC {
	private:
		_SPRAY();
		int	Bottle_ref, Bottle_P;
		int	Air_ref, Air_P;
		int	idx,simrate,timeout,count;

	public:
		static _SPRAY* instance;
		virtual void		Newline(void);
		virtual FRESULT	Decode(char *);
		virtual int			Fkey(int);
		void		Increment(int, int);

		mode		mode;
		_IOC_SprayAck	IOC_SprayAck;

		_VALVE	*BottleIn,*BottleOut,*Air,*Water;
		int			AirLevel, WaterLevel;
		int			Status(void);
		void		LoadSettings(FILE *);
		void		SaveSettings(FILE *);
		void		Increment(int, int);
	
		_LCD		*lcd;
		_PLOT<unsigned short>  plot;	
	
		bool		Simulator(void);
		double	pComp,pBott,pNozz,Pin,Pout;
};

#endif
	
