#ifndef RTC_H
#define RTC_H
#include "stm32f4xx_hal.h"
#include "term.h"

class	_RTC : public _TERM {
	private:
		int	idx;
		RTC_TimeTypeDef time;
		RTC_DateTypeDef date;

	public:
		_RTC();
		~_RTC();
		_io *io;
		virtual void	Newline(void);
		virtual int		Fkey(int);
		void					Increment(int, int);
};

#endif
