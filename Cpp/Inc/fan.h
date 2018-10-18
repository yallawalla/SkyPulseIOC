#ifndef		FAN_H
#define		FAN_H

#include	"term.h"
#include	"adc.h"
#include	"err.h"
#include	"ff.h"
#include	<algorithm>
#define	__ramp(x,x1,x2,y1,y2)	std::min(std::max(((y2-y1)*(x-x1))/(x2-x1)+y1,y1),y2)

extern	TIM_HandleTypeDef htim10;

class	_FAN : public _TERM, public _ADC {
	private:
		int fpl,fph,ftl,fth;
		int	idx,timeout,speed,tacho_limit,__fanTacho;
	public:
		_FAN();
		void		LoadSettings(FIL *);
		void		SaveSettings(FIL *);

		virtual void	Newline(void);
		virtual int		Fkey(int);
		void		Increment(int, int);
		int			Rpm(int),mode;
		bool		Setup(void);
		_err		Status(void);
};

#endif
