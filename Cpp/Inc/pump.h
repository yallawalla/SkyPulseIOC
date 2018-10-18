#ifndef		PUMP_H
#define		PUMP_H

#include	"term.h"
#include	"adc.h"
#include	"err.h"
#include	"ff.h"
#include	<algorithm>

#define		__ramp(x,x1,x2,y1,y2)	std::min(std::max(((y2-y1)*(x-x1))/(x2-x1)+y1,y1),y2)

extern 		DAC_HandleTypeDef hdac;

class	_PUMP : public _TERM, public _ADC {
	private:
		int 		fpl,fph,ftl,fth;
		int			idx,timeout,__pumpTacho,__flowTacho;
		int			speed, tacho_limit,curr_limit, flow_limit;

	public:
		_PUMP();
		void		LoadSettings(FIL *);
		void		SaveSettings(FIL *);

		virtual void	Newline(void);
		virtual int		Fkey(int);
		void		Increment(int, int);
		void		Enable(void),Disable(void);
		int			Rpm(int),Flow,mode;
		bool		Setup(void),Enabled(void);
		_err		Status(void);
};

#endif
