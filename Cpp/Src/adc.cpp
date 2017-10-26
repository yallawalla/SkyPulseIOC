#include	"adc.h"
#include	"misc.h"
adc	_ADC::val	={0},
		_ADC::fval={0},
		_ADC::offset={0},
		_ADC::gain={1};
/*******************************************************************************
* Function Name	:
* Description		: 
* Output				:
* Return				: None
*******************************************************************************/
_ADC::_ADC() {
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&val, sizeof(val)/sizeof(uint16_t));
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
void	_ADC::adcSmooth() {
	fval.T1					+= (val.T1					- fval.T1)/16;
	fval.T2					+= (val.T2					- fval.T2)/16;
	fval.V5					+= (val.V5					- fval.V5)/16;
	fval.V12				+= (val.V12					- fval.V12)/16;
	fval.V24				+= (val.V24					- fval.V24)/16;
	fval.cooler			+= (val.cooler			- fval.cooler)/16;
	fval.bottle			+= (val.bottle			- fval.bottle)/16;
	fval.compressor	+= (val.compressor	- fval.compressor)/16;
	fval.air				+= (val.air					- fval.air)/16;
	fval.Ipump			+= (val.Ipump				- fval.Ipump)/16;
}
/*******************************************************************************
* Function Name	:
* Description		: 
* Output				:
* Return				: None
*******************************************************************************/
_err _ADC::adcError() {
_err e=_NOERR;
				
		if(HAL_GetTick() > _ADC_ERR_DELAY) {
			if(abs(fval.V5  - _V5to16X)	> _V5to16X/10)
				e = (_err)(e | _V5);
			if(abs(fval.V12 - _V12to16X) > _V12to16X/5)
				e = (_err)(e | _V12);
			if(abs(fval.V24 - _V24to16X) > _V24to16X/10)
				e = (_err)(e | _V24);
			if(Th2o() > 50*100)
				e = (_err)(e | _sysOverheat);
			}		
		return e;
	}
