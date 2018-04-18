/**
	******************************************************************************
	* @file		spray.cpp
	* @author	Fotona d.d.
	* @version
	* @date		
	* @brief	Timers initialization & ISR
	*
	*/

/** @addtogroup
* @{
*/
#include	"ioc.h"
/*******************************************************************************
* Function Name				:
* Description					: 
* Output							:
* Return							: None
*******************************************************************************/
int		_VALVE::Set() { 
			return valve_drive[n];
};
void	_VALVE::Set(int i) { 
			valve_drive[n]=i;
};
void	_VALVE::Set(int i, int t) { 
			valve_drive[n]=i;
			valve_timeout[n]=__time__+t;
};
/*******************************************************************************
* Function Name				:
* Description					: 
* Output							:
* Return							: None
*******************************************************************************/
_SPRAY::_SPRAY() {	
			BottleOut=	new _VALVE(0,false);
			BottleIn=		new _VALVE(1,true);
			Air=				new _VALVE(2,true);
			Water=			new _VALVE(3,true);

			BottleIn->Close();
			BottleOut->Close();
			Air->Close();
			Water->Close();
		
			offset.air=offset.bottle=offset.compressor=	_BAR(1);
			gain.air=																		_BAR(2.5);
			gain.bottle=																_BAR(0.5);
			gain.compressor=														_BAR(1);
			waterGain=																	_BAR(1.2);
			Air_P=Bottle_P=0;
			AirLevel=WaterLevel=0;
			Bottle_ref=Air_ref=													_BAR(1);

			mode.Simulator=false;
			mode.Vibrate=false;
			mode.Water=false;
			mode.Air=false;
			idx=0;
			
			readyTimeout=0;
			offsetTimeout=__time__ + 5000;

			simrate=0;
			lcd=NULL;
			Pin=4.0;
			pComp= pBott=pNozz=Pout=1.0;
}
/*******************************************************************************
* Function Name				:
* Description					: 
* Output							:
* Return							: None
*******************************************************************************/
#define	_P_THRESHOLD  0x8000
#define	_A_THRESHOLD	0x2000

_err	_SPRAY::Status() {
_err	_err=_NOERR;
//------------------------------------------------------------------------------
			if(offsetTimeout && __time__ > offsetTimeout) {
				offset.bottle += fval.air - offset.air;
				offset.air = fval.air;
				offsetTimeout=0;
			}
//------------------------------------------------------------------------------
			Air_ref			= offset.air + AirLevel*gain.air/10;
			Bottle_ref	= offset.bottle + (Air_ref - offset.air)*waterGain/0x10000 + gain.bottle*WaterLevel/10;
//			Bottle_ref	= offset.bottle + AirLevel*waterGain*(100+4*WaterLevel)/100/10;
//------------------------------------------------------------------------------
			if(AirLevel || WaterLevel) {
				Bottle_P += (Bottle_ref - (int)fval.bottle)/16;
				if(Bottle_P < -_P_THRESHOLD) {
					Bottle_P=0;
					BottleIn->Close();
					BottleOut->Open(120);
					if(readyTimeout)
						readyTimeout = __time__ + _SPRAY_READY_T;
				}
				if(Bottle_P > _P_THRESHOLD) {
					Bottle_P=0;
					BottleIn->Open(120);
					BottleOut->Close();
					if(readyTimeout)
						readyTimeout = __time__ + _SPRAY_READY_T;
				}
			} else {
					BottleIn->Close();
					BottleOut->Open();
			}

			if(10*(fval.compressor-offset.compressor)/gain.compressor < 25)
				_err = _err | _sprayInPressure;		
			if(readyTimeout && __time__ < readyTimeout)
				_err = _err |  _sprayNotReady;
			else
				readyTimeout=0;

			if(mode.Water)
				Water->Open();
			else
				Water->Close();	

			if(AirLevel && mode.Air) {
				Air_P += (Air_ref-(int)fval.air);
				Air_P=std::max(0,std::min(_A_THRESHOLD*__PWMRATE, Air_P));
				if(mode.Vibrate && __time__ % 50 < 10)
					Air->Open();
				else
					Air->Set(Air_P/_A_THRESHOLD);						
			}
			else
				Air->Close();

			if(mode.Simulator && Simulator()) {
#ifdef USE_LCD
			if(lcd && plot.Refresh())
				lcd->Grid();
#endif
			}
			return _err;
}
//_________________________________________________________________________________
void	_SPRAY::Newline(void) {
			if(mode.Simulator) {
				printf("\r:spray %3d,%5d,%5.2lf,%5.2lf,%5.2lf",
					AirLevel,WaterLevel,
						(double)(fval.air-offset.air)/_BAR(1),
							(double)(fval.bottle-offset.bottle)/_BAR(1),
								Pout-1.0);
			} else {
				printf("\r:spray %3d,%5d,%5.2lf,%5.2lf,%5.2lf",
					AirLevel,WaterLevel,
						(double)(fval.air-offset.air)/_BAR(1),
							(double)(fval.bottle-offset.bottle)/_BAR(1),
								(double)(fval.compressor-offset.compressor)/_BAR(1));
			}
			if(mode.Air) 
				printf("   Air"); 
			else 
				printf("   ---"); 
			if(mode.Water) 
				printf(" Water"); 
			else 
				printf("   ---"); 
			
			for(int i=1+6*(6-idx); i--; printf("\b"));		
			Repeat(200,__CtrlR);
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
int		_SPRAY::Fkey(int t) {
			switch(t) {
					case __f7:
					case __F7:
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
					case __CtrlR:
					Increment(0,0);
					break;
					case __PageUp:
						waterGain=std::min(waterGain+1000,_BAR(2.5));
						break;
					case __PageDown:
						waterGain=std::max(waterGain-1000,_BAR(0.5));
						break;					
					case __CtrlV:
						if(mode.Vibrate)
							mode.Vibrate=false;
						else
							mode.Vibrate=true;
						break;
					case __CtrlI:
						_ADC::offset.air = _ADC::fval.air;
						_ADC::offset.bottle = _ADC::fval.bottle;
						printf("\r\n: air/water offset.... \r\n:");
						break;
						case __CtrlS:
							HAL_ADC_DeInit(&hadc1);
							lcd=new _LCD;
							lcd->Add(&_ADC::fval.compressor,_BAR(1.0),_BAR(0.02), LCD_COLOR_YELLOW);
							lcd->Add(&_ADC::fval.bottle,_BAR(1.0),_BAR(0.02), LCD_COLOR_GREY);
							lcd->Add(&_ADC::fval.air,_BAR(1.0),_BAR(0.02), LCD_COLOR_MAGENTA);
							mode.Simulator=true;
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
void	_SPRAY::LoadSettings(FIL *f) {
char	c[128];
			f_gets(c,sizeof(c),f);
			sscanf(c,"%hu,%hu,%hu,%hu",&offset.cooler,&offset.bottle,&offset.compressor,&offset.air);
			f_gets(c,sizeof(c),f);
			sscanf(c,"%hu,%hu,%hu,%hu",&gain.cooler,&gain.bottle,&gain.compressor,&gain.air);
			f_gets(c,sizeof(c),f);
			sscanf(c,"%d",&waterGain);
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
void	_SPRAY::SaveSettings(FIL *f) {
			f_printf(f,"%5d,%5d,%5d,%5d                 /.. offset\r\n", offset.cooler, offset.bottle, offset.compressor, offset.air);
			f_printf(f,"%5d,%5d,%5d,%5d                 /.. gain\r\n", gain.cooler,gain.bottle,gain.compressor, gain.air);
			f_printf(f,"%5d                                   /.. gain\r\n", waterGain);
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
void	_SPRAY::Increment(int a, int b) {
			idx= std::min(std::max(idx+b,0),6);
			switch(idx) {
				case 0:
					AirLevel 			= std::min(std::max(0,AirLevel+a),10);
					readyTimeout	= __time__ + _SPRAY_READY_T;
					break;
				case 1:
					WaterLevel 		= std::min(std::max(0,WaterLevel+a),10);
					readyTimeout	= __time__ + _SPRAY_READY_T;
					break;
						case 2:
							break;
				case 3:
					gain.bottle		= std::min(std::max(_BAR(0.1),gain.bottle+100*a),_BAR(0.5));
					break;
				case 4:
					if(mode.Simulator) {
						Pout 				= std::min(std::max(0.5,Pout+(double)a/10.0),1.5);
						if(a) {
							AirLevel = WaterLevel;
							mode.Air = mode.Water = false;
							offsetTimeout = __time__ + 3000;
						}
					}
					break;
				case 5:
					if(a < 0)
						mode.Air=false;
					else if(a > 0)
						mode.Air=true;
					break;
				case 6:
					if(a < 0)
						mode.Water=false;
					else if(a > 0)
						mode.Water=true;
					break;
			}
			Newline();
}	
/*******************************************************************************
* Function Name	:
* Description		: 
* Output				:
* Return				: 
********************************************************************************

                         ///------R2----Uc2-----///-----Rout----Pout
                          |              |
                          |             ///
                          |              |
         Pin-----Rin-----Uc1              \____Rw_____
                          |                            \
                          |                            Uc3-----Rsp----Pout
                         XXX___________________Ra______/

*******************************************************************************/
bool	_SPRAY::Simulator(void) {
#define Uc1	 pComp
#define Uc2	 pBott
#define Uc3	 pNozz

#define Rin		10
#define Rout	100

#define R2		100
#define Rw 		300
#define Ra 		300
#define Rsp		10
#define C1 		1e-2
#define C3		100e-6
#define C2 		50e-3
#define dt		1e-3

double	Iin=(Pin-Uc1)/Rin;
double	I12=(Uc1-Uc2)/R2;
double	I13=(Uc1-Uc3)/Ra;
double	I23=(Uc2-Uc3)/Rw;
double	I3=(Uc3-Pout)/Rsp;
double	Iout=(Uc2 - Pout)/Rout;

	I13 = Air->Set() * I13/__PWMRATE;

	if(BottleIn->Closed()) {
		I12=0;
		if(I23 < 0)
			lcd->Colour(&_ADC::fval.bottle,LCD_COLOR_GREEN);
		else
			lcd->Colour(&_ADC::fval.bottle,LCD_COLOR_GREY);
	} else
		lcd->Colour(&_ADC::fval.bottle,LCD_COLOR_RED);

	if(BottleOut->Closed())
		Iout=0;
	else
		lcd->Colour(&_ADC::fval.bottle,LCD_COLOR_BLUE);

	if(Water->Closed())
		I23=0;

	Uc1 += (Iin-I12-I13)/C1*dt;
	Uc2 += (I12-I23-Iout)/C2*dt;
	Uc3 += (I23+I13-I3)/C3*dt;	

	fval.compressor	=_BAR(pComp);
	fval.bottle			=_BAR(pBott + 0.05*I12*R2);
	fval.air					=_BAR(pNozz + I13*Ra);

	fval.V5	= _V5to16X;
	fval.V12	= _V12to16X;
	fval.V24	= _V24to16X;

	fval.T2=(unsigned short)0xafff;
	if(simrate && __time__ < simrate)
		return false;
	simrate = __time__ + 10;
	return true;
}



