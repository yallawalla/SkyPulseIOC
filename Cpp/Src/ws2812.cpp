/**
	******************************************************************************
	* @file		leds.cpp
	* @author	Fotona d.d.
	* @version
	* @date
	* @brief	WS2812B driver class
	*
	*/
	
/** @addtogroup
* @{
*/

#include	"ws2812.h"
#include	"proc.h"
#include	"misc.h"
#include	"math.h"
#include	"string.h"
#include	"stdio.h"

_WS	*_WS::instance=NULL;
ws2812 _WS::ws[] = 
			{{8,{0,0,0},NULL,noCOMM,NULL},
			{24,{0,0,0},NULL,noCOMM,NULL},
			{8,{0,0,0},NULL,noCOMM,NULL},
			{8,{0,0,0},NULL,noCOMM,NULL},
			{24,{0,0,0},NULL,noCOMM,NULL},
			{8,{0,0,0},NULL,noCOMM,NULL},
			{0,{0,0,0},NULL,noCOMM,NULL}};
/*******************************************************************************/
/**
	* @brief	_WS class constructor
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
_WS::~_WS() {
			_proc_remove((void *)proc_WS2812,this);
			delete dma_buf;
			for(int i=0; ws[i].size; ++i)
				delete ws[i].cbuf;
}
/*******************************************************************************/
/**
	* @brief	_WS class constructor
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
_WS::_WS()  {
			
//
// ________________________________________________________________________________
			ws2812 *w=ws;
			int i=0;
			while(w->size)																		// count number of leds
				i+=w++->size;
			dma_buf=(dma *)led_drive;													// allocate dma buffer
			dma_size=80*24-2;	i*sizeof(dma)/sizeof(short)/2;						// polovica zaradi 2xbursta
			
			w=ws;
			i=0;
			while(w->size) {
				w->cbuf=new HSV_set[w->size];										// alloc color buffer
				w->lbuf=&dma_buf[i];													// pointer to dma tab
				i+=w++->size;
			}
			_proc_add((void *)proc_WS2812,this,(char *)"WS2812",10);
			idx=idxled=0;
		}
/*******************************************************************************/
/**
	* @brief	_WS trigger method
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
void		_WS::trigger() {
int			i,j,k;
dma			*p;
RGB_set	q;
int			imax=sizeof(ws)/sizeof(ws2812);

				for(i=0; i<imax/2; ++i) {
					for(j=0; j<ws[i].size; ++j) {
						if(ws[i].cbuf) {
							HSV2RGB(ws[i].cbuf[j], &q);
							for(k=0,p=ws[i].lbuf; k<8; ++k) {
								(q.b & (0x80>>k)) ? (p[j].b[k][0]=53)	: (p[j].b[k][0]=20);
								(q.g & (0x80>>k)) ? (p[j].g[k][0]=53)	: (p[j].g[k][0]=20);
								(q.r & (0x80>>k)) ? (p[j].r[k][0]=53)	: (p[j].r[k][0]=20);
							}
						}
							else
								for(k=0,p=ws[i].lbuf; k<24; ++k)
									p[j].g[k][0]=20;

						if(ws[i+imax/2].cbuf) {
							HSV2RGB(ws[i+imax/2].cbuf[j], &q);
							for(k=0,p=ws[i].lbuf; k<8; ++k) {
								(q.b & (0x80>>k)) ? (p[j].b[k][1]=53)	: (p[j].b[k][1]=20);
								(q.g & (0x80>>k)) ? (p[j].g[k][1]=53)	: (p[j].g[k][1]=20);
								(q.r & (0x80>>k)) ? (p[j].r[k][1]=53)	: (p[j].r[k][1]=20);
							}
						}
							else
								for(k=0,p=ws[i].lbuf; k<24; ++k)
									p[j].g[k][1]=20;
				}
			}		
			HAL_TIM_PWM_Start_DMA(&htim4,TIM_CHANNEL_1,(uint32_t *)led_drive, __LEDS*24+2);

}
/*******************************************************************************/
/**
	* @brief	_WS class periodic task
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
void		*_WS::proc_WS2812(_WS *me) {
int			j,k,trg=0;
ws2812	*w=ws;
//------------------------------------------------------------------------------
				do {
					HSV_set	color = w->color;
					k=0;
//------------------------------------------------------------------------------
					switch(w->mode) {
						case noCOMM:
							break;
//------------------------------------------------------------------------------
						case FILL_OFF:
							color.v=0;
						case FILL_ON:
							for(j=k=0; j<w->size;++j) {
								w->cbuf[j].h = color.h;
								w->cbuf[j].s = color.s;

								if(w->cbuf[j].v < color.v)
									w->cbuf[j].v += (color.v - w->cbuf[j].v)/10+1;
								else if(w->cbuf[j].v > color.v)
									w->cbuf[j].v -= (w->cbuf[j].v - color.v)/10+1;
								else
									++k;
							}
						break;
//------------------------------------------------------------------------------
						case SWITCH_OFF:
							color.v=0;
						case SWITCH_ON:
							for(j=k=0; j<w->size;++j) {
								w->cbuf[j].h = color.h;
								w->cbuf[j].s = color.s;
								w->cbuf[j].v = color.v;
							}
							k=w->size;
						break;
//------------------------------------------------------------------------------
						case FILL_RIGHT_OFF:
							color.v=0;
						case FILL_RIGHT_ON:
							j=w->size; 
							k=0;
							while(--j) {
								w->cbuf[j].h = w->cbuf[j-1].h;
								w->cbuf[j].s = w->cbuf[j-1].s;
								
								if(w->cbuf[j].v < w->cbuf[j-1].v)
									w->cbuf[j].v += (w->cbuf[j-1].v - w->cbuf[j].v)/4+1;
								else if(w->cbuf[j].v > w->cbuf[j-1].v)
									w->cbuf[j].v -= (w->cbuf[j].v - w->cbuf[j-1].v)/4+1;
								else
									++k;
							}
							w->cbuf[j].h = color.h;
							w->cbuf[j].s = color.s;
							if(w->cbuf[j].v < color.v)
								w->cbuf[j].v += (color.v - w->cbuf[j].v)/4+1;
							else if(w->cbuf[j].v > color.v)
								w->cbuf[j].v -= (w->cbuf[j].v - color.v)/4+1;
							else
								++k;
							break;
//------------------------------------------------------------------------------
						case FILL_LEFT_OFF:
							color.v=0;
						case FILL_LEFT_ON:
							for(j=k=0; j<w->size-1;++j) {
								w->cbuf[j].h = w->cbuf[j+1].h;
								w->cbuf[j].s = w->cbuf[j+1].s;
								
								if(w->cbuf[j].v < w->cbuf[j+1].v)
									w->cbuf[j].v += (w->cbuf[j+1].v - w->cbuf[j].v)/4+1;
								else if(w->cbuf[j].v > w->cbuf[j+1].v)
									w->cbuf[j].v -= (w->cbuf[j].v - w->cbuf[j+1].v)/4+1;
								else
									++k;
							}
							w->cbuf[j].h = color.h;
							w->cbuf[j].s = color.s;
							if(w->cbuf[j].v < color.v)
								w->cbuf[j].v += (color.v - w->cbuf[j].v)/4+1;
							else if(w->cbuf[j].v > color.v)
								w->cbuf[j].v -= (w->cbuf[j].v - color.v)/4+1;
							else
								++k;
							break;
//------------------------------------------------------------------------------
						case RUN_RIGHT_OFF:
							color.v=0;
						case RUN_RIGHT_ON:
							for(j=k=0; j<w->size-1;++j) {
								if(w->cbuf[j] != w->cbuf[j+1])
									w->cbuf[j] = w->cbuf[j+1];
								else
									++k;
							}
							if(w->cbuf[j] != color)
								w->cbuf[j] = color;
							else
								++k;
							break;
//------------------------------------------------------------------------------
						case RUN_LEFT_OFF:
							color.v=0;
						case RUN_LEFT_ON:
							j=w->size; 
							k=0;
							while(--j) {
								if(w->cbuf[j] != w->cbuf[j-1])
									w->cbuf[j] = w->cbuf[j-1];
								else
									++k;
							}
							if(w->cbuf[j] != color)
								w->cbuf[j] = color;
							else
								++k;
							break;
//------------------------------------------------------------------------------
						default:
							break;
						}
//------------------------------------------------------------------------------					
						if(w->mode != noCOMM) {
							++trg;
							if(k==w->size)
								w->mode=noCOMM;
						}
				} while((++w)->size);

				if(trg)
					me->trigger();
				return NULL;
}
//______________________________________________________________________________________
int			strscan(char *s,char *ss[],int c) {
				int		i=0;
				while(1)
				{
					while(*s==' ') ++s;
					if(!*s)
						return(i);

					ss[i++]=s;
					while(*s && *s!=c)
					{
						if(*s==' ')
							*s='\0';
						s++;
					}
					if(!*s)
						return(i);
					*s++=0;
				}
}
/*******************************************************************************/
/**
	* @brief	_WS parser, initial '.' character
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
int			_WS::ColorOn(char *c) {
char		*p=strtok(c," ,");
				switch(*p) {
//________________________________________________
					case '0':
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
						do {
							ws[atoi(p)].mode=SWITCH_ON;
							p=strtok(NULL," ,");
							} while(p);
						break;
//________________________________________________
					case 'f':
						for(p=strtok(NULL," ,"); p; p=strtok(NULL,","))
							ws[atoi(p)].mode=FILL_ON;
						break;
//________________________________________________
					case 'l':
						for(p=strtok(NULL," ,"); p; p=strtok(NULL,","))
							ws[atoi(p)].mode=RUN_LEFT_ON;
						break;
//________________________________________________
					case 'r':
						for(p=strtok(NULL," ,"); p; p=strtok(NULL,","))
							ws[atoi(p)].mode=RUN_RIGHT_ON;
						break;
//________________________________________________

					default:
						return FR_INVALID_PARAMETER;
				}
			return FR_OK;
			}	
/*******************************************************************************/
/**
	* @brief	_WS parser, initial '.' character
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
int		_WS::ColorOff(char *c) {
char	*p=strtok(c," ,");
			switch(*p) {
//______________________________________________
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				do {
						ws[atoi(p)].mode=SWITCH_OFF;
						p=strtok(NULL," ,");
						} while(p);
					break;
//______________________________________________
				case 'f':
					for(p=strtok(NULL," ,"); p; p=strtok(NULL,","))
						ws[atoi(p)].mode=FILL_OFF;
					break;
//______________________________________________
				case 'l':
					for(p=strtok(NULL," ,"); p; p=strtok(NULL,","))
						ws[atoi(p)].mode=RUN_LEFT_OFF;
					break;
//______________________________________________
				case 'r':
					for(p=strtok(NULL," ,"); p; p=strtok(NULL,","))
						ws[atoi(p)].mode=RUN_RIGHT_OFF;
					break;
//______________________________________________
				default:
					return FR_INVALID_PARAMETER;
				}
			return FR_OK;
}	
/*******************************************************************************/
/**
	* @brief	_WS class load/save settings method
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
int		_WS::SetColor(char *c) {
int		i;
			c=strtok(c,", ");
			switch(*c) {
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
					i=atoi(c);
					ws[i].color.h =atoi(strtok(NULL,", "));
					ws[i].color.s =atoi(strtok(NULL,", "));
					ws[i].color.v =atoi(strtok(NULL,", "));
					break;						
				case 't':
					i=atoi(strtok(NULL,", "));
					if(i<5 || i>1000)
						return FR_INVALID_PARAMETER;
					_proc_find((void *)proc_WS2812,this)->dt=i;	
					break;
				default:
					return FR_INVALID_PARAMETER;
				}
			return FR_OK;
}
/*******************************************************************************/
/**
	* @brief	_WS class load/save settings method
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
int		_WS::GetColor(int color) {
	
			_TERM key;
			ws2812 *w=&ws[color];
			int flag=0;
			
			printf("\n\rHSB:%d,%d,%d      ",w->color.h,w->color.s,w->color.v);
			while(1) {
				switch(key.Escape()) {
					case EOF:
						break;
					case __Up:
						++flag;
					w->color.h=std::min(359,w->color.h + 1);
						break;				
					case __Down:
						++flag;
						w->color.h=std::max(1,w->color.h - 1);
						break;
					case __Right:
						++flag;
						w->color.s=std::min(255,w->color.s + 1);
						break;				
					case __Left:
						++flag;
						w->color.s=std::max(1,w->color.s - 1);
						break;				
					case __PageUp:
						++flag;
						w->color.v=std::min(255,w->color.v + 1);
						break;				
					case __PageDown:
						++flag;
						w->color.v=std::max(1,w->color.v - 1);
						break;				
					case __Esc:
						return FR_OK;
				}
				if(flag) {
					printf("\rHSB:%d,%d,%d      ",w->color.h,w->color.s,w->color.v);
					for(int i=0; i<w->size; ++i)
						w->cbuf[i] =w->color;
					trigger();
					flag=0;
				}
				_wait(10,_proc_loop);
			}	
				}
/*******************************************************************************/
/**
	* @brief	_WS class load/save settings method
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
void		_WS::SaveSettings(FILE *f){
				for(int i=0; ws[i].size; ++i)
					fprintf(f,"=color %d,%d,%d,%d\r\n",i,ws[i].color.h,ws[i].color.s,ws[i].color.v);
}
/*******************************************************************************
 * Function RGB2HSV
 * Description: Converts an RGB color value into its equivalen in the HSV color space.
 * Copyright 2010 by George Ruinelli
 * The code I used as a source is from http://www.cs.rit.edu/~ncs/color/t_convert.html
 * Parameters:
 *   1. struct with RGB color (source)
 *   2. pointer to struct HSV color (target)
 * Notes:
 *   - r, g, b values are from 0..255
 *   - h = [0,360], s = [0,255], v = [0,255]
 *   - NB: if s == 0, then h = 0 (undefined)
 ******************************************************************************/
void _WS::RGB2HSV(RGB_set RGB, HSV_set *HSV){
 unsigned char min, max, delta;
 
 if(RGB.r<RGB.g)min=RGB.r; else min=RGB.g;
 if(RGB.b<min)min=RGB.b;
 
 if(RGB.r>RGB.g)max=RGB.r; else max=RGB.g;
 if(RGB.b>max)max=RGB.b;
 
 HSV->v = max;                // v, 0..255
 
 delta = max - min;                      // 0..255, < v
 
 if( max != 0 )
 HSV->s = (int)(delta)*255 / max;        // s, 0..255
 else {
 // r = g = b = 0        // s = 0, v is undefined
 HSV->s = 0;
 HSV->h = 0;
 return;
 }
 
 if( RGB.r == max )
 HSV->h = (RGB.g - RGB.b)*60/delta;        // between yellow & magenta
 else if( RGB.g == max )
 HSV->h = 120 + (RGB.b - RGB.r)*60/delta;    // between cyan & yellow
 else
 HSV->h = 240 + (RGB.r - RGB.g)*60/delta;    // between magenta & cyan
 
 if( HSV->h < 0 )
 HSV->h += 360;
}
/*******************************************************************************
 * Function HSV2RGB
 * Description: Converts an HSV color value into its equivalen in the RGB color space.
 * Copyright 2010 by George Ruinelli
 * The code I used as a source is from http://www.cs.rit.edu/~ncs/color/t_convert.html
 * Parameters:
 *   1. struct with HSV color (source)
 *   2. pointer to struct RGB color (target)
 * Notes:
 *   - r, g, b values are from 0..255
 *   - h = [0,360], s = [0,255], v = [0,255]
 *   - NB: if s == 0, then h = 0 (undefined)
 ******************************************************************************/
void _WS::HSV2RGB(HSV_set HSV, RGB_set *RGB){
 int i;
 float f, p, q, t, h, s, v;
 
 h=(float)HSV.h;
 s=(float)HSV.s;
 v=(float)HSV.v;
 
 s /=255;
 
 if( s == 0 ) { // achromatic (grey)
 RGB->r = RGB->g = RGB->b = v;
 return;
 }
 
 h /= 60;            // sector 0 to 5
 i = floor( h );
 f = h - i;            // factorial part of h
 p = (unsigned char)(v * ( 1 - s ));
 q = (unsigned char)(v * ( 1 - s * f ));
 t = (unsigned char)(v * ( 1 - s * ( 1 - f ) ));
 
 switch( i ) {
 case 0:
 RGB->r = v;
 RGB->g = t;
 RGB->b = p;
 break;
 case 1:
 RGB->r = q;
 RGB->g = v;
 RGB->b = p;
 break;
 case 2:
 RGB->r = p;
 RGB->g = v;
 RGB->b = t;
 break;
 case 3:
 RGB->r = p;
 RGB->g = q;
 RGB->b = v;
 break;
 case 4:
 RGB->r = t;
 RGB->g = p;
 RGB->b = v;
 break;
 default:        // case 5:
 RGB->r = v;
 RGB->g = p;
 RGB->b = q;
 break;
 }
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
void _WS::Newline(void) {
	printf("\r:color n,HSV %4d,%3d,%3d,%3d",idxled,ws[idxled].color.h,ws[idxled].color.s,ws[idxled].color.v);
		for(int i=1+4*(3-idx); i--; printf("\b"));
	trigger();
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
int		_WS::Fkey(int t) {
			switch(t) {
					case __f10:
					case __F10:
						return __F12;
					case __Up:
						Increment(1,0);
					break;
					case __Down:
						Increment(-1,0);
					break;
					case __Left:
						Increment(0,-1);
					break;
					case __Right:
						Increment(0,1);
					break;
				}
			return EOF;
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/ 
/*******************************************************************************/
void	_WS::Increment(int a, int b) {
			idx= std::min(std::max(idx+b,0),3);
			switch(idx) {
				case 0:
					idxled= std::min(std::max(idxled+a,0),5);
					break;
				case 1:
					ws[idxled].color.h=std::min(std::max(ws[idxled].color.h+a,0),359);
					break;
				case 2:
					ws[idxled].color.s=std::min(std::max(ws[idxled].color.s+a,0),255);
					break;
				case 3:
					ws[idxled].color.v=std::min(std::max(ws[idxled].color.v+a,0),255);
					break;
			}
			Newline();
}		

/**
* @}

.c 0,120,255,50 
.c 5,120,255,50 
.c 1,180,180,50
.c 4,180,180,50
.c 2,7,255,50
.c 3,7,255,50

.t 30
.c 2,7,255,50 
.c 3,7,255,50
.l 2,2
.r 3,2
.c 1,180,180,50 
.c 4,180,180,50
.r 1,2
.r 4,2
.d 3000
.c 2,7,255,0 
.c 3,7,255,0
.r 2,2
.l 3,2
.c 1,180,180,0 
.c 4,180,180,0
.l 1,2
.l 4,2
.d 3000

.c 2,7,255,50 
.c 3,7,255,50
.f 2,0
.f 3,0
.d 100
.c 2,7,255,0 
.c 3,7,255,0
.f 2,0
.f 3,0
.d 100
.c 2,7,255,50 
.c 3,7,255,50
.f 2,0
.f 3,0
.d 100
.c 2,7,255,0 
.c 3,7,255,0
.f 2,0
.f 3,0
.d 100
.c 2,7,255,50 
.c 3,7,255,50
.f 2,0
.f 3,0
.d 100
.c 2,7,255,0 
.c 3,7,255,0
.f 2,0
.f 3,0


.c 2,7,255,50 
.c 3,7,255,50
.f 2,2
.f 3,2
.c 1,180,180,50 
.c 4,180,180,50
.l 1,2
.l 4,2
.d 25
.c 1,180,180,0 
.c 4,180,180,0
.l 1,2
.l 4,2
.c 2,7,255,0 
.c 3,7,255,0
.f 2,2
.f 3,2

=color 0,180,180,95
=color 1,180,180,95
=color 2,7,255,95
=color 3,7,255,95
=color 4,180,180,95
=color 5,180,180,95

=color 0,180,180,50
=color 1,180,180,50
=color 2,7,255,50
=color 3,7,255,50
=color 4,180,180,50
=color 5,180,180,50

+c f,2
-c f,3
w 100
-c f,2
+c f,3
w 100
+c f,2
-c f,3
w 100
-c f,2
+c f,3
w 100
+c f,2
-c f,3
w 100
-c f,2
+c f,3
w 100
+c f,2
-c f,3
w 100
-c f,2
+c f,3
w 100
+c f,2
-c f,3
w 100
-c f,2
+c f,3
w 100
+c f,2
-c f,3
w 100
-c f,2
+c f,3
w 100
-c f,2
-c f,3

-c r,4
+c r,4

*/


