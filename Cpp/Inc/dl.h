#ifndef		DL_H
#define		DL_H

#include "err.h"
#include "term.h"
#include "adc.h"
#include "misc.h"
//_____________________________________________________________________
typedef __packed struct {
	uint16_t	Pavg:14,ch:2;
	uint32_t	on:24,off:24;
} DL_Timing;
//_____________________________________________________________________
typedef __packed struct {
	uint16_t	l0,l1,l2;
	uint8_t		ch0:2,ch1:2,ch2:2;
} DL_Limits;
//_____________________________________________________________________
typedef	__packed struct {
	uint32_t 	on,off;
	uint32_t	val;
	uint8_t		mode:2;
} limit;
//_____________________________________________________________________
class	_DL  : public _TERM {
		private:
			bool						selected;			
			float						offset[2];
			uint16_t				dma[154][2];
			uint32_t				ton,toff,active,ref[2];
			lopass					high, filter, max;
			int32_t					idx,dlscale[2],dacScale,dacOffset;
			limit						limits[3];
			
		public:
			static _DL* instance;
			_DL();
		
			_err		Status(bool);
			void		filterCbk(bool);
			void		setActiveCh(uint32_t);
			void 		Setup();
			void 		Setup(DL_Timing *);
			void 		Setup(DL_Limits *);

			_err 		Check(float, float);
		
			int			stest_delay,stest_err,selftest(void);
		
			void		LoadSettings(FIL *);
			void		SaveSettings(FIL *);

			void		Increment(int, int);
			virtual void	Newline(void);
			virtual int Fkey(int);
};

#define	dac(a,b) do \
										if(_TERM::debug & (1<<a)) \
											HAL_DAC_SetValue(&hdac,DAC_CHANNEL_2,DAC_ALIGN_12B_R, std::max(0,std::min(0xfff,(int)(dacScale*(b) + dacOffset)))); \
										while(0)
#endif
