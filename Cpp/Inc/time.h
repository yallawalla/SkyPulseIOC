#ifndef TIME_H
#define TIME_H
#include "stm32f4xx_hal.h"
#include "term.h"

class	_TIME : public _TERM {
	private:
		int	idx;
	public:
		_TIME();
		virtual void	Newline(void);
		virtual int		Fkey(int);
		void					Increment(int, int);
};

#endif
