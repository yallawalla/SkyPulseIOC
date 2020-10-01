#ifndef		DL_H
#define		DL_H

#include "err.h"
#include "term.h"
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
class lopass {
	private:
		class _lopass {
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
		} ch1,ch2;
	public:
		float val[2];
		lopass(float fo, float fs) : ch1(fo,fs),ch2(fo,fs) {
			val[0]=val[1]=0;
		}
		void eval(float in0,float in1) {
			val[0]=ch1.eval(in0);
			val[1]=ch2.eval(in1);
		}	
		void reset() {
			ch1.reset();
			ch2.reset();
			val[0]=val[1]=0;
		}	
};

class	_DL  : public _TERM {
		private:
			bool						selected, emit;
			float 					offset[2],ref[2];
			unsigned short	dma[154][2];
			unsigned int 		errtout[2],ton,toff;
			lopass					high, filter;
			int							idx,dlscale[2],dacScale,dacOffset;
			limit						limits[3];
			uint8_t					count;
			
		public:
			static _DL* instance;
			_DL();
		
			_err		Status(bool);
			void		filterCbk(bool);
		
			
			uint8_t	setActiveCh(uint8_t);
		
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
										 HAL_DAC_SetValue(&hdac,DAC_CHANNEL_2,DAC_ALIGN_12B_R, dacScale*(b) + dacOffset); \
										while(0)
#endif
