#include	"adc.h"
#include	"dl.h"
#include	"misc.h"

adc		_ADC::val[16]	={},
			_ADC::fval={},
			_ADC::offset={},
			_ADC::gain={};
/*******************************************************************************
* Function Name	: ADC constructor
* Description		: 2-APB2 clock divider
*								: 4-ADC prescaler
*								: 26-sample cycles, 12-conversion cycles
*								: 2 channels concversion
*								: 154 pairs DMA block
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
			return (__fit(fval.T1,Rtab,Ttab) + __fit(fval.T2,Rtab,Ttab))/2;
}
/*******************************************************************************
* Function Name	:
* Description		: 
* Output				:
* Return				: None
*******************************************************************************/
int		_ADC::Th2o(int n) {
				if(n)
					return __fit(fval.T2,Rtab,Ttab);
				else
					return __fit(fval.T1,Rtab,Ttab);
}
/*******************************************************************************
* Function Name	:
* Description		: 
* Output				:
* Return				: None
*******************************************************************************/
void	_ADC::adcFilter() {
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
					_DL::instance->filterCbk(1);
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void	HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc) {
			if(hadc==&hadc2)
					_DL::instance->filterCbk(0);
}
/*******************************************************************************
* Function Name	:
* Description		: 
* Output				:
* Return				: None
*******************************************************************************/
_err _ADC::adcError() {
_err	e=_NOERR;
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
			if(Th2o() > 55*100)
				e = e | _sysOverheat;
			if(fval.T1 > 0xf000 ||  fval.T2 > 0xf000 || abs(fval.T1  - fval.T2)	> 0x0a00)
				e = e | _TsenseError;
			}		
		return e;
	}
