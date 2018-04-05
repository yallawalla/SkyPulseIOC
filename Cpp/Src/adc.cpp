#include	"adc.h"
#include	"misc.h"

diode	_ADC::dsense[16*10]	={};
adc		_ADC::val[16]	={},
			_ADC::fval={},
			_ADC::offset={},
			_ADC::gain={};
/*******************************************************************************
* Function Name	:
* Description		: 
* Output				:
* Return				: None
*******************************************************************************/
_ADC::_ADC() {
			HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&val, sizeof(val)/sizeof(uint16_t));
			HAL_ADC_Start_DMA(&hadc2, (uint32_t*)&dsense, sizeof(dsense)/sizeof(uint16_t));
}
/*******************************************************************************
* Function Name	:
* Description		: 
* Output				:
* Return				: None
*******************************************************************************/
int		_ADC::Th2o() {
			return __fit(fval.T1,Rtab,Ttab);
}
/*******************************************************************************
* Function Name	:
* Description		: 
* Output				:
* Return				: None
*******************************************************************************/
void	_ADC::adcFilter() {
//		fval.T1					+= (val.T1					- fval.T1)/16;
//		fval.T2					+= (val.T2					- fval.T2)/16;
//		fval.V5					+= (val.V5					- fval.V5)/16;
//		fval.V12				+= (val.V12					- fval.V12)/16;
//		fval.V24				+= (val.V24					- fval.V24)/16;
//		fval.cooler			+= (val.cooler			- fval.cooler)/16;
//		fval.bottle			+= (val.bottle			- fval.bottle)/16;
//		fval.compressor	+= (val.compressor	- fval.compressor)/16;
//		fval.air				+= (val.air					- fval.air)/16;
//		fval.Ipump			+= (val.Ipump				- fval.Ipump)/16;
//		fval.diode1			+= (val.diode1			- fval.diode1)/16;
//		fval.diode2			+= (val.diode2			- fval.diode2)/16;
			memset(&fval,0,sizeof(fval));
			for(int i=0; i<sizeof(val)/sizeof(adc); ++i) {
				fval.T1					+= val[i].T1;
				fval.T2					+= val[i].T2;
				fval.V5					+= val[i].V5;
				fval.V12				+= val[i].V12;
				fval.V24				+= val[i].V24;
				fval.cooler			+= val[i].cooler;
				fval.bottle			+= val[i].bottle;
				fval.compressor	+= val[i].compressor;
				fval.air				+= val[i].air;
				fval.Ipump			+= val[i].Ipump;
			}
}


float x,dx,k=0.0006250;
float fo=15e6/(12+56)/2;

// double t = Ton / (1 - exp(-fo * k * Ton));
// 9.06666666666666666666666 us
								
void	_ADC::diodeFilter(int half) {
diode	*d=dsense;
int		n=sizeof(dsense)/sizeof(diode)/2;
			if(half)
				d=&dsense[n];
	
//			pump_drive=x;
	
			
			while(n--) {
				dx += k*(d++->diode1 - x-dx*(float)2);
				x += k*dx;	
			}
//			pump_drive=0;
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void	HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
			if(hadc==&hadc1)
					_ADC::adcFilter();
			if(hadc==&hadc2)
					_ADC::diodeFilter(1);
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void	HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc) {
			if(hadc==&hadc2)
					_ADC::diodeFilter(0);
}
/*******************************************************************************
* Function Name	:
* Description		: 
* Output				:
* Return				: None
*******************************************************************************/
_err _ADC::adcError() {
_err e=_NOERR;

		if(_SYS_SHG_ENABLED && !_cwbBUTTON) {
			if(!_cwbENGM)
				e = e | _handpcDisabled;
			else if(!_cwbDOOR)
				e = e | _doorswDisabled;
			else
				e = e | _emgDisabled;
		}
		
		if(__time__ > _ADC_ERR_DELAY) {
			if(abs(fval.V5  - _V5to16X)	> _V5to16X/10)
				e = e | _V5;
			if(abs(fval.V12 - _V12to16X) > _V12to16X/5)
				e = e | _V12;
			if(abs(fval.V24 - _V24to16X) > _V24to16X/10)
				e = e | _V24;
			if(Th2o() > 50*100)
				e = e | _sysOverheat;
			}		
		return e;
	}
