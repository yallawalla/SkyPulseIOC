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
		float Val[2],Max[2],Min[2];
		unsigned int timeout;
		lopass(float fo, float fs) : ch1(fo,fs),ch2(fo,fs) {
			Min[0]=Min[1]=Max[0]=Max[1]=Val[0]=Val[1]=0;
			timeout=0;
		}
		void eval(float in0,float in1) {
			Val[0]=ch1.eval(in0);
			Val[1]=ch2.eval(in1);
		}	
		void reset() {
			ch1.reset();
			ch2.reset();
			Val[0]=Val[1]=0;
		}	
};

class	_DL  : public _TERM {
		private:
			bool	active;
			float offset[2],
						max[2],
						ref[2];
			unsigned short	dma[154][2];
			unsigned int 		ton,toff;
			unsigned int 		on,off;
			lopass	filter, filterRef;

		public:
			static _DL* instance;
			_DL();
			_err		Status(bool);
			void		filterCbk(int);
			void 		setTiming(int,int);
			void 		setLimits(int,int);
		
			void		LoadSettings(FIL *);
			void		SaveSettings(FIL *);

			virtual void	Newline(void);
			virtual int		Fkey(int);
};
#endif
