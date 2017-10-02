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
#include	"spray.h"
#include	"ioc.h"
_SPRAY		*_SPRAY::instance=NULL;
/*******************************************************************************
* Function Name				:
* Description					: 
* Output							:
* Return							: None
*******************************************************************************/
int	_VALVE::Set() { 
		return valve_drive[n];
};
void	_VALVE::Set(int i) { 
		valve_drive[n]=i;
};
void	_VALVE::Set(int i, int t) { 
		valve_drive[n]=i;
		valve_time[n]=HAL_GetTick()+t;
};
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
		gain.air=																		_BAR(2);
		gain.bottle=																_BAR(1.3);
		gain.compressor=														_BAR(1);

		Air_P=Bottle_P=0;
		AirLevel=WaterLevel=0;
		Bottle_ref=Air_ref=													_BAR(1);

		mode.Simulator=false;
		mode.Vibrate=false;
		mode.On=false;
		idx=0;

		simrate=timeout=count=0;
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
#define   _P_THRESHOLD  0x8000
#define		_A_THRESHOLD	0x2000

_Error	_SPRAY::Status(void *v) {
	_IOC *parent=static_cast<_IOC *>(v);

_Error	e=_NOERR;

		if(AirLevel) {
			Bottle_P += (Bottle_ref - (int)val.bottle)/16;
			if(Bottle_P < -_P_THRESHOLD) {
				Bottle_P=0;
				BottleIn->Close();
				BottleOut->Open(150);
				timeout = HAL_GetTick() + 500;
				++count;
			}
			if(Bottle_P > _P_THRESHOLD) {
				Bottle_P=0;
				BottleIn->Open(150);
				BottleOut->Close();
				timeout = HAL_GetTick() + 500;
				++count;
			}
		} else {
				BottleIn->Close();
				BottleOut->Open();
		}
		
		Air_ref			= offset.air + AirLevel*gain.air/10;
		Bottle_ref	= offset.bottle + AirLevel*gain.bottle*(100+4*WaterLevel)/100/10;		

		if(count == 5) {
			parent->IOC_SprayAck.Status = _SPRAY_NOT_READY;
			parent->IOC_SprayAck.Send();
			++count;
		}
		
		if(HAL_GetTick() > timeout) {
			if(count > 5) {
				parent->IOC_SprayAck.Status = _SPRAY_READY;
				parent->IOC_SprayAck.Send();
			}
			count=0;
		}						

		if(WaterLevel && mode.On)
			Water->Open();
		else
			Water->Close();	

		if(AirLevel && mode.On) {
			Air_P += (Air_ref-(int)val.air);
			Air_P=std::max(0,std::min(_A_THRESHOLD*__PWMRATE, Air_P));
			if(mode.Vibrate && HAL_GetTick() % 50 < 10)
				Air->Open();
			else
				Air->Set(Air_P/_A_THRESHOLD);
			
			if(abs(fval.compressor - 4*offset.compressor) > offset.compressor/2)
				e = _sprayInPressure;
		}
		else
			Air->Close();
		
		if(mode.Simulator && Simulator()) {

		if(lcd->Refresh())
			lcd->Grid();

		}		
		return e;
}
//_________________________________________________________________________________
void _SPRAY::Newline(void) {
		if(mode.Simulator) {
			printf("\r:air/water    %3d,%3d,%3.1lf,%3.1lf",
				AirLevel,WaterLevel,
					Pin,Pout);
			for(int i=1+4*(3-idx);i--;printf("\b"));							
		} else {
			printf("\r:air/water    %3d,%3d,%3.1lf",
				AirLevel,WaterLevel,
					(double)(fval.compressor-offset.compressor)/gain.compressor);
			for(int i=1+4*(2-idx);i--;printf("\b"));							
		}	
}
//_________________________________________________________________________________
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
					case __Esc:
						mode.On ? mode.On=false: mode.On=true;
					break;		
					case __CtrlS:
						HAL_ADC_DeInit(&hadc1);
						lcd=new _LCD;
						lcd->Add(&_ADC::val.compressor,_BAR(1.0),_BAR(0.02), LCD_COLOR_YELLOW);
						lcd->Add(&_ADC::val.bottle,_BAR(1.0),_BAR(0.02), LCD_COLOR_GREY);
						lcd->Add(&_ADC::val.air,_BAR(1.0),_BAR(0.02), LCD_COLOR_MAGENTA);
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
void	_SPRAY::LoadSettings(FILE *f) {
char	c[128];
			fgets(c,sizeof(c),f);
			sscanf(c,"%hu,%hu,%hu,%hu",&offset.cooler,&offset.bottle,&offset.compressor,&offset.air);
			fgets(c,sizeof(c),f);
			sscanf(c,"%hu,%hu,%hu,%hu",&gain.cooler,&gain.bottle,&gain.compressor,&gain.air);
			fgets(c,sizeof(c),f);
			sscanf(c,"%d,%d",&AirLevel,&WaterLevel);
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
void	_SPRAY::SaveSettings(FILE *f) {
			fprintf(f,"%5d,%5d,%5d,%5d                 /.. offset\r\n", offset.cooler, offset.bottle, offset.compressor, offset.air);
			fprintf(f,"%5d,%5d,%5d,%5d                 /.. gain\r\n", gain.cooler, gain.bottle, gain.compressor, gain.air);
			fprintf(f,"%5d,%5d                             /.. air, H2O\r\n", AirLevel, WaterLevel);
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
void	_SPRAY::Increment(int a, int b) {
			if(mode.Simulator)
				idx= std::min(std::max(idx+b,0),3);
			else
				idx= std::min(std::max(idx+b,0),1);
			
			switch(idx) {
				case 0:
					AirLevel 		= std::min(std::max(0,AirLevel+a),10);
					break;
				case 1:
					WaterLevel 	= std::min(std::max(0,WaterLevel+a),10);
					break;
				case 2:
					Pin 				= std::min(std::max(0.5,Pin+(double)a/10.0),4.5);
					break;
				case 3:
					Pout 				= std::min(std::max(0.5,Pout+(double)a/10.0),1.5);
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
		lcd->Colour(&_ADC::val.bottle,LCD_COLOR_GREY);
	} else
		lcd->Colour(&_ADC::val.bottle,LCD_COLOR_RED);

	if(BottleOut->Closed())
		Iout=0;
	else
		lcd->Colour(&_ADC::val.bottle,LCD_COLOR_BLUE);

	if(Water->Closed())
		I23=0;

	Uc1 += (Iin-I12-I13)/C1*dt;
	Uc2 += (I12-I23-Iout)/C2*dt;
	Uc3 += (I23+I13-I3)/C3*dt;	

	val.compressor	=_BAR(pComp);
	val.bottle			=_BAR(pBott + 0.05*I12*R2);
	val.air					=_BAR(pNozz + I13*Ra);

	val.V5	= _V5to16X;
	val.V12	= _V12to16X;
	val.V24	= _V24to16X;

	val.T2=(unsigned short)0xafff;
	if(simrate && HAL_GetTick() < simrate)
		return false;
	simrate = HAL_GetTick() + 10;
	return true;
}



