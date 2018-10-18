#ifndef					WS2812_H
#define					WS2812_H
#include				"stm32f4xx_hal.h"
#include				"cli.h"
#include 				<algorithm>
typedef struct	{uint8_t r; uint8_t g; uint8_t b; }	RGB;
typedef struct	{int16_t h; uint8_t s; uint8_t v; }	HSV;
inline bool operator == (HSV &a, HSV &b) {
    return a.h == b.h && a.s == b.s && a.v == b.v;
}

inline bool operator != (HSV &a, HSV &b) {
    return !(a==b);
}

typedef enum		{ noCOMM,
										SWITCH_ON, SWITCH_OFF, 
										MOD_ON, MOD_OFF, 
										FILL_ON, FILL_OFF, 
										FILL_LEFT_ON, FILL_RIGHT_ON, 
										FILL_LEFT_OFF, FILL_RIGHT_OFF,
										RUN_LEFT_ON, RUN_RIGHT_ON,
										RUN_LEFT_OFF, RUN_RIGHT_OFF
								}	wsCmd;

typedef	struct	{
	uint8_t	size;						// N of leds in element				
	HSV			color;					// base color of element
	HSV			*hsvp;					// color buffer, size of elemnt
	wsCmd		mode;						// mode of animation
	HSV			mod;						// modulation hsv components
	int			shift;					// phase shift
} ws2812;

class	_WS : public _CLI {
	private:
		int			idx,idxled,busy;
		void 		RGB2HSV( RGB, HSV *);
		void		HSV2RGB( HSV, RGB *);
		void		trigger(void);
	
		static 	ws2812 	ws[];
		static	HSV	HSVbuf[];
		static	uint8_t	modh, mods,modv,modt;
		_proc		*parse;
	
	public:
		_WS(void);
		~_WS(void);
		virtual	FRESULT	Batch(char *);
		virtual	FRESULT	Decode(char *);
		virtual void		Newline(void);
		virtual int			Fkey(int);
		void						Increment(int, int);
		void						LoadSettings(FIL *);
		void						SaveSettings(FIL *);
	
		FRESULT					ColorOn(char *);
		FRESULT					ColorOff(char *);
		FRESULT					ColorSet(char *);
		void						Cmd(int,wsCmd);
		static void			*proc_WS2812(_WS *);
};


#define __rearmDMA(len)																																												\
	do {																																																				\
			__HAL_DMA_DISABLE(htim4.hdma[TIM_DMA_ID_UPDATE]);																												\
			__HAL_TIM_DISABLE(&htim4);																																							\
			__HAL_TIM_SET_COUNTER(&htim4,0);																																				\
			while(htim4.hdma[TIM_DMA_ID_UPDATE]->Instance->CR & DMA_SxCR_EN);																				\
			htim4.hdma[TIM_DMA_ID_UPDATE]->Instance->NDTR=len;																											\
			__HAL_DMA_CLEAR_FLAG(htim4.hdma[TIM_DMA_ID_UPDATE],																											\
						DMA_FLAG_HTIF2_6 | DMA_FLAG_TEIF2_6 | DMA_FLAG_DMEIF2_6	| DMA_FLAG_FEIF2_6 | DMA_FLAG_TCIF2_6);		\
			__HAL_DMA_ENABLE(htim4.hdma[TIM_DMA_ID_UPDATE]);																												\
			__HAL_TIM_ENABLE(&htim4);																																								\
	} while(0)
	
#endif
