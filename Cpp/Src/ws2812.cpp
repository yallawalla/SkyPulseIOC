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
#include	"ctype.h"

HSV			_WS::HSVbuf[__LEDS];
ws2812	_WS::ws[] = 
			{{8,{0,255,50},  NULL,noCOMM,{0,0,0},0},
			{24,{60,255,50}, NULL,noCOMM,{0,0,0},0},
			{8,{120,255,50}, NULL,noCOMM,{0,0,0},0},
			{8,{180,255,50}, NULL,noCOMM,{0,0,0},0},
			{24,{240,255,50},NULL,noCOMM,{0,0,0},0},
			{8,{300,255,50}, NULL,noCOMM,{0,0,0},0}};

/*******************************************************************************/
/**
	* @brief	_WS class constructor
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
#define __IMAX sizeof(ws)/sizeof(ws2812)
_WS::_WS()  {
int			k=0;
				for(int i=0; i< __IMAX; ++i) {
					ws[i].hsvp=&HSVbuf[k];
					k+= ws[i].size;	
				}
				parse=_proc_add((void *)proc_WS2812,this,(char *)"WS2812",20);
				idx=idxled=busy=0;
				io->put=NULL;
}
/*******************************************************************************/
/**
	* @brief	_WS class constructor
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
_WS::~_WS() {
}
/*******************************************************************************/
/**
	* @brief	_WS trigger method
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
void		_WS::trigger() {
RGB			rgbL,rgbR;
HSV			*hsvL=ws[0].hsvp,
				*hsvR=ws[__IMAX/2].hsvp;
	
uint16_t	k,*p=led_drive;

				while(hsvL != ws[__IMAX/2].hsvp) {
					HSV2RGB(*hsvL++, &rgbL);
					HSV2RGB(*hsvR++, &rgbR);
					for(k=0; k<8; ++k) {
						(rgbL.g & (0x80>>k)) ? (*p++=__TH)	: (*p++=__TL);
						(rgbR.g & (0x80>>k)) ? (*p++=__TH)	: (*p++=__TL);
					}
					for(k=0; k<8; ++k) {
						(rgbL.r & (0x80>>k)) ? (*p++=__TH)	: (*p++=__TL);
						(rgbR.r & (0x80>>k)) ? (*p++=__TH)	: (*p++=__TL);
					}
					for(k=0; k<8; ++k) {
						(rgbL.b & (0x80>>k)) ? (*p++=__TH)	: (*p++=__TL);
						(rgbR.b & (0x80>>k)) ? (*p++=__TH)	: (*p++=__TL);
					}
				}
				__rearmDMA(p-led_drive+2);
}
/*******************************************************************************/
/**
	* @brief	_WS class periodic task
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
void		*_WS::proc_WS2812(_WS *me) {
int			j,k;
ws2812	*w=ws;
				me->busy=0;
//------------------------------------------------------------------------------
				do {
					HSV	color = w->color;
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
								w->hsvp[j].h = color.h;
								w->hsvp[j].s = color.s;

								if(w->hsvp[j].v < color.v)
									w->hsvp[j].v += (color.v - w->hsvp[j].v)/10+1;
								else if(w->hsvp[j].v > color.v)
									w->hsvp[j].v -= (w->hsvp[j].v - color.v)/10+1;
								else
									++k;
							}
						break;
//------------------------------------------------------------------------------
						case SWITCH_OFF:
							color.v=0;
						case SWITCH_ON:
							for(j=k=0; j<w->size;++j) {
								w->hsvp[j].h = color.h;
								w->hsvp[j].s = color.s;
								w->hsvp[j].v = color.v;
							}
							k=w->size;
						break;
//------------------------------------------------------------------------------
						case MOD_OFF:
							color.v=0;
						case MOD_ON:
							for(j=k=0; j<w->size;++j) {
								w->hsvp[j].h = (color.h + (int)(w->mod.h * cos(2.0 * _PI * (float)(j+w->shift) / (float)w->size))) % 360;
								w->hsvp[j].s = color.s + w->mod.s * cos(2.0 * _PI * (float)(j+w->shift) / (float)w->size);
								w->hsvp[j].v = color.v + w->mod.v * cos(2.0 * _PI * (float)(j+w->shift) / (float)w->size);
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
								w->hsvp[j].h = w->hsvp[j-1].h;
								w->hsvp[j].s = w->hsvp[j-1].s;
								
								if(w->hsvp[j].v < w->hsvp[j-1].v)
									w->hsvp[j].v += (w->hsvp[j-1].v - w->hsvp[j].v)/4+1;
								else if(w->hsvp[j].v > w->hsvp[j-1].v)
									w->hsvp[j].v -= (w->hsvp[j].v - w->hsvp[j-1].v)/4+1;
								else
									++k;
							}
							w->hsvp[j].h = color.h;
							w->hsvp[j].s = color.s;
							if(w->hsvp[j].v < color.v)
								w->hsvp[j].v += (color.v - w->hsvp[j].v)/4+1;
							else if(w->hsvp[j].v > color.v)
								w->hsvp[j].v -= (w->hsvp[j].v - color.v)/4+1;
							else
								++k;
							break;
//------------------------------------------------------------------------------
						case FILL_LEFT_OFF:
							color.v=0;
						case FILL_LEFT_ON:
							for(j=k=0; j<w->size-1;++j) {
								w->hsvp[j].h = w->hsvp[j+1].h;
								w->hsvp[j].s = w->hsvp[j+1].s;
								
								if(w->hsvp[j].v < w->hsvp[j+1].v)
									w->hsvp[j].v += (w->hsvp[j+1].v - w->hsvp[j].v)/4+1;
								else if(w->hsvp[j].v > w->hsvp[j+1].v)
									w->hsvp[j].v -= (w->hsvp[j].v - w->hsvp[j+1].v)/4+1;
								else
									++k;
							}
							w->hsvp[j].h = color.h;
							w->hsvp[j].s = color.s;
							if(w->hsvp[j].v < color.v)
								w->hsvp[j].v += (color.v - w->hsvp[j].v)/4+1;
							else if(w->hsvp[j].v > color.v)
								w->hsvp[j].v -= (w->hsvp[j].v - color.v)/4+1;
							else
								++k;
							break;
//------------------------------------------------------------------------------
						case RUN_RIGHT_OFF:
							color.v=0;
						case RUN_RIGHT_ON:
							for(j=k=0; j<w->size-1;++j) {
								if(w->hsvp[j] != w->hsvp[j+1])
									w->hsvp[j] = w->hsvp[j+1];
								else
									++k;
							}
							if(w->hsvp[j] != color)
								w->hsvp[j] = color;
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
								if(w->hsvp[j] != w->hsvp[j-1])
									w->hsvp[j] = w->hsvp[j-1];
								else
									++k;
							}
							if(w->hsvp[j] != color)
								w->hsvp[j] = color;
							else
								++k;
							break;
//------------------------------------------------------------------------------
						default:
							break;
						}
//------------------------------------------------------------------------------					
						if(w->mode != noCOMM) {
							++me->busy;
							if(k==w->size)
								w->mode=noCOMM;
						}
				} while(++w != &ws[__IMAX]);

				if(me->busy)
					me->trigger();
				return NULL;
}
/*******************************************************************************/
/**
	* @brief	_WS parser, initial '.' character
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
FRESULT	_WS::ColorOn(char *p) {
char		*c=strchr(p,'+');
				*c++=0;
				while(*p==' ')	++p;
				p=strtok(p,",");
				while(p) {
					if(!isdigit(*p))
						return FR_INVALID_PARAMETER;
					switch(*c) {
						case 0:
							ws[atoi(p)].mode = SWITCH_ON;
						break;
						case 'm':
							ws[atoi(p)].mode = MOD_ON;
						break;
						case 'f':
							ws[atoi(p)].mode = FILL_ON;
						break;
						case 'l':
							ws[atoi(p)].mode = RUN_LEFT_ON;
						break;
						case 'r':
							ws[atoi(p)].mode = RUN_RIGHT_ON;
						break;
						default:
							return FR_INVALID_PARAMETER;
					}
					p=strtok(NULL,",");
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
FRESULT	_WS::ColorOff(char *p) {
char		*c=strchr(p,'-');
				*c++=0;
				while(*p==' ')	++p;
				p=strtok(p,",");
				while(p) {
					if(!isdigit(*p))
						return FR_INVALID_PARAMETER;
					switch(*c) {
						case 0:
							ws[atoi(p)].mode=SWITCH_OFF;
						break;
						case 'm':
							ws[atoi(p)].mode=MOD_OFF;
						break;
						case 'f':
							ws[atoi(p)].mode=FILL_OFF;
						break;
						case 'l':
							ws[atoi(p)].mode=RUN_LEFT_OFF;
						break;
						case 'r':
							ws[atoi(p)].mode=RUN_RIGHT_OFF;
						break;
						default:
							return FR_INVALID_PARAMETER;
					}
					p=strtok(NULL,",");
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
FRESULT	_WS::ColorSet(char *p) {
char		*c=strchr(p,'=');
				*c++=0;
				while(*p==' ')	++p;
				p=strtok(p,",");
				switch(*p) {
					case 't':
						parse->dt=atoi(c);	
					break;
					default:
						while(p) {
							if(!isdigit(*p))
								return FR_INVALID_PARAMETER;
							int i=atoi(p);
							if(sscanf(c,"%hu,%hhu,%hhu",&ws[i].color.h, &ws[i].color.s, &ws[i].color.v) != 3)
								return FR_INVALID_PARAMETER;
							p=strtok(NULL,",");
						}				
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
void		_WS::SaveSettings(FIL *f){
				for(int i=0; i < __IMAX; ++i)
					f_printf(f,"color %d=%d,%d,%d\r\n",i,ws[i].color.h,ws[i].color.s,ws[i].color.v);
}
/*******************************************************************************/
/**
	* @brief	_WS class load/save settings method
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
void		_WS::LoadSettings(FIL *f){
char		c[32];
				for(int i=0; i < __IMAX; ++i) {
					f_gets(c,sizeof(c),f);
					ColorSet(strchr(c,' '));
				}
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
void 		_WS::RGB2HSV(RGB RGB, HSV *HSV){
 unsigned char min, max, delta;
 
 if(RGB.r<RGB.g)min=RGB.r; else min=RGB.g;
 if(RGB.b<min)min=RGB.b;
 
 if(RGB.r>RGB.g)max=RGB.r; else max=RGB.g;
 if(RGB.b>max)max=RGB.b;
 
 HSV->v = max;                			// v, 0..255
 
 delta = max - min;									// 0..255, < v
 
 if( max != 0 )
 HSV->s = (int)(delta)*255 / max;		// s, 0..255
 else {
 // r = g = b = 0										// s = 0, v is undefined
 HSV->s = 0;
 HSV->h = 0;
 return;
 }
 
 if( RGB.r == max )
 HSV->h = (RGB.g - RGB.b)*60/delta;					// between yellow & magenta
 else if( RGB.g == max )
 HSV->h = 120 + (RGB.b - RGB.r)*60/delta;		// between cyan & yellow
 else
 HSV->h = 240 + (RGB.r - RGB.g)*60/delta;		// between magenta & cyan
 
 if(HSV->h < 0 )
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
void 		_WS::HSV2RGB(HSV HSV, RGB *RGB){
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
void		_WS::Newline(void) {
				_print("\r:color n,HSV %4d,%3d,%3d,%3d,%3d,%3d,%3d,%3d",
					idxled,ws[idxled].color.h,ws[idxled].color.s,ws[idxled].color.v,
						ws[idxled].mod.h,ws[idxled].mod.s,ws[idxled].mod.v,ws[idxled].shift);
					for(int i=1+4*(7-idx); i--; _print("\b"));
				_wait(20);
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
int			_WS::Fkey(int t) {
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
					default:
						return t;
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
FRESULT	_WS::Decode(char *c) {
//				while(busy)
//					_wait(2);
				if(strchr(c,'@'))
					return _TERM::Batch(++c);
				if(!strncmp(c,"wait",4)) {
					c=strchr(c,' ');
					_wait(atoi(c));
					return FR_OK;
				}
// preskoci "col" (v cli ga pobere ze parser, v batch je se noter...) 
				if(!strncmp(c,"col",3))
					c=strchr(c,' ');
				
				if(strchr(c,'='))
					return ColorSet(c);
				if(strchr(c,'+'))
					return ColorOn(c);
				if(strchr(c,'-'))
					return ColorOff(c);	
				return FR_INVALID_PARAMETER;
}
/*******************************************************************************
* Function Name	:
* Description		:
* Output				:
* Return				:
*******************************************************************************/
FRESULT _WS::Batch(char *filename) {
				_buffer_push(io->rx,filename,strlen(filename));
				_buffer_push(io->rx,(char *)"\r",1);
				return FR_OK;
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/ 
/*******************************************************************************/
void		_WS::Increment(int a, int b) {
				idx= std::min(std::max(idx+b,0),7);
				switch(idx) {
					case 0:
						idxled= std::min(std::max(idxled+a,0),5);
						break;
					case 1:
						ws[idxled].color.h = (ws[idxled].color.h+a) % 360;
						if(ws[idxled].color.h < 0)
							ws[idxled].color.h=359;
						break;
					case 2:
						ws[idxled].color.s=std::min(std::max(ws[idxled].color.s+a,0),255);
						break;
					case 3:
						ws[idxled].color.v=std::min(std::max(ws[idxled].color.v+a,0),255);
						break;
					case 4:
						ws[idxled].mod.h=std::min(std::max(ws[idxled].mod.h+a,0),180);
						break;
					case 5:
						ws[idxled].mod.s=std::min(std::max(ws[idxled].mod.s+a,0),180);
						break;
					case 6:
						ws[idxled].mod.v=std::min(std::max(ws[idxled].mod.v+a,0),180);
						break;
					case 7:
						ws[idxled].shift=std::min(std::max(ws[idxled].shift+a,0),100);
						break;
				}
				Newline();
				ws[idxled].mode = MOD_ON;
				trigger();
}		
/**
* @}
*/


