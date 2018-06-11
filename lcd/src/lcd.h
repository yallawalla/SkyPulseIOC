#ifndef		LCD_H
#define		LCD_H
#include	"stm32f4xx.h"
#include	"stm32f4_discovery_lcd.h"
#include	<vector>
using					namespace std;
template			<typename Type>	

class	_PLOT {
	class _POINT {
		public:
			Type 		*Plot,Offset,Scale;
			short		Colour;
			void Draw(int x) {
				LCD_SetTextColor(Colour);
				LCD_DrawCircle(x,LCD_PIXEL_HEIGHT/2+100-((*Plot-Offset)/Scale),1);
			}
		};

	private:
		int x, idx;
		vector<_POINT> points;
	
	public:
		_PLOT() {
			x=idx=0;
		};

		~_PLOT() {
		};

		void	Add(Type *type, Type offset, Type scale, short colour) {
			_POINT p;
			p.Plot=type;
			p.Offset=offset;
			p.Scale=scale;
			p.Colour=colour;
			points.push_back(p);
		};
		
		void	Clear(void) {
			points.clear();
		}

		bool	Refresh(void) {
			for(int i = 0; i != points.size(); ++i)
				points[i].Draw(x);
			x = ++x % LCD_PIXEL_WIDTH;
			
			if(x) 
				return false;
			else
				return true;
		}

		void	Colour(Type *type, short colour) {
			for(int i = 0; i != points.size(); ++i)
				if(points[i].Plot==type) {
					points[i].Colour=colour;
					return;
				}
		}
};



class	_LCD {
		private:
			short x,y;

		public:
			_LCD();
		
void	Home(void);
void	Grid(void);

};

#endif
