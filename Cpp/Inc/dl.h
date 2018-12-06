#ifndef		DL_H
#define		DL_H

#include "err.h"
#include "term.h"
#include "misc.h"

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
			bool						active,synced;
			float 					offset[2];
			unsigned short	dma[154][2];
			unsigned int 		ton,toff,timeout[2],ref[2];
			unsigned int 		on,off,lim[2],mode;
			lopass					high, filter, filterRef;
			int							idx,dlscale[2];

		public:
			static _DL* instance;
			_DL();
			_err		Status(bool);
			void		filterCbk(int);
			void 		setTiming(int,int);
			void 		setLimits(int,int,int);
		
			void		LoadSettings(FIL *);
			void		SaveSettings(FIL *);

			void		Increment(int, int);
			virtual void	Newline(void);
			virtual int		Fkey(int);
};
#endif
