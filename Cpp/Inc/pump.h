#ifndef		PUMP_H
#define		PUMP_H
#include	<stdio.h>
#include	"term.h"
#include	"adc.h"
#include	"err.h"
#include	"ff.h"
#include	<algorithm>
#define	__ramp(x,x1,x2,y1,y2)	std::min(std::max(((y2-y1)*(x-x1))/(x2-x1)+y1,y1),y2)

extern DAC_HandleTypeDef hdac;

class	_PUMP : public _TERM, public _ADC {
	private:
		int fpl,fph,ftl,fth,idx;
	public:
		_PUMP();
		void	LoadSettings(FILE *);
		void	SaveSettings(FILE *);

		virtual void		Newline(void);
		virtual int			Fkey(int);
		void		Increment(int, int);
		int			Rpm(int);
		_Error	Status(void);
};

#endif
