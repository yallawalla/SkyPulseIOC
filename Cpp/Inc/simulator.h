#ifndef		SIMULATOR_H
#define		SIMULATOR_H
#include	"adc.h"
#include	"lcd.h"

class _SIMULATOR : public _ADC {
private:
	_PLOT<unsigned short>  plot;	
	int		rate;
	float	pComp,
				pBott,
				pNozz,
				tau1,
				tau2;
public:
	float	Pin,
				Pext;
	_SIMULATOR();
	~_SIMULATOR();
	_LCD *lcd;

	void Poll(void *);
};
#endif
