#ifndef					WS2812_H
#define					WS2812_H
#include				"stm32f4xx_hal.h"
#include				"term.h"
#include 				<algorithm>
typedef struct	{unsigned char r; unsigned char g; unsigned char b; }	RGB_set;
typedef struct	{
	signed short	h;
	unsigned char s,v;
}	HSV_set;

inline bool operator == (HSV_set &a, HSV_set &b) {
    return a.h == b.h && a.s == b.s && a.v == b.v;
}

inline bool operator != (HSV_set &a, HSV_set &b) {
    return !(a==b);
}

typedef enum		{ noCOMM,
										SWITCH_ON, SWITCH_OFF, 
										FILL_ON, FILL_OFF, 
										FILL_LEFT_ON, FILL_RIGHT_ON, 
										FILL_LEFT_OFF, FILL_RIGHT_OFF,
										RUN_LEFT_ON, RUN_RIGHT_ON,
										RUN_LEFT_OFF, RUN_RIGHT_OFF
								}	wsCmd;

typedef struct	{
	uint16_t g[8][2];					// green
	uint16_t r[8][2];					// red
	uint16_t b[8][2];					// blue
} dma;

typedef	struct	{
	int				size;						// N of leds in element				
	HSV_set		color, *cbuf;		// color of element, color buffer, size of elemnt
	wsCmd			mode;						// mode of animation
	dma 			*lbuf;					// pointer to dma buffer
} ws2812;

class	_WS : public _TERM {
	private:
		int			idx,idxled;
		void 		RGB2HSV( RGB_set, HSV_set *);
		void		HSV2RGB( HSV_set, RGB_set *);
		void		trigger(void);
		dma			*dma_buf;
		int			dma_size;
		static 	ws2812 	ws[];
		_WS(void);
		~_WS(void);
	public:
		static _WS*		instance;
		virtual void	Newline(void);
		virtual int		Fkey(int);
		void					Increment(int, int);
	
		int						ColorOn(char *);
		int						ColorOff(char *);
		int						SetColor(char *);
		int						GetColor(int);
		void					Cmd(int,wsCmd);
		void					SaveSettings(FILE *);
		static void		*proc_WS2812(_WS *);
		static _WS		*InstanceOf() {
										if(instance==NULL)
											instance=new _WS();
										return instance;
		}
};

#endif
