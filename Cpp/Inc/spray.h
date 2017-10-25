#ifndef		SPRAY_H
#define		SPRAY_H

#include	<stdio.h>
#include	"term.h"
#include	"adc.h"
#include	"err.h"
#include	"ff.h"
#include	<algorithm>
#include	"lcd.h"

#define		_BAR(a) ((a)*16384.0)
				
typedef	struct {
	bool	On:1;
	bool	Vibrate:1;
	bool	Simulator:1;
	bool	Ready:1;
}	mode;
//________________________________________________________________________________________________
class	_VALVE {
	private:
		int 	n;
		bool	inv;
	public:
		_VALVE(int k, bool i) {	
			n = k; 
			inv=i;
		};
		
		int	 Set();
		void Set(int);
		void Set(int, int);

		bool Opened(void)							{ return inv ? Set() : !Set();};
		bool Closed(void)							{ return !Opened();};
		void Open(void)								{ inv ? Set(__PWMRATE): Set(0);};
		void Close(void)							{ inv ? Set(0): Set(__PWMRATE);};
		void Open(int i)							{ inv ? Set(__PWMRATE,i): Set(0,i);};
		void Close(int i)							{ inv ? Set(0,i): Set(__PWMRATE,i);};
};

class	_SPRAY : public _TERM, public _ADC {
	private:
		int	Bottle_ref, Bottle_P;
		int	Air_ref, Air_P;
		int	idx,simrate,timeout,count;

	public:
		_SPRAY();
		void		LoadSettings(FILE *);
		void		SaveSettings(FILE *);
		virtual void		Newline(void);
		virtual int			Fkey(int);
		void		Increment(int, int);

		_VALVE	*BottleIn,*BottleOut,*Air,*Water;
		_Error	Status(void *);
		mode		mode;

		int			AirLevel, WaterLevel;
		_LCD		*lcd;
	
	
		bool		Simulator(void);
		double	pComp,pBott,pNozz,Pin,Pout;
};

#endif
	
