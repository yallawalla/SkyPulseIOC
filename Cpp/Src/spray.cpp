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
		
			offset.air=offset.bottle=offset.compressor=	_BAR(1.0f);
			gain.air=																		_BAR(1.5f);
			gain.bottle=																_BAR(0.5f);
			gain.compressor=														_BAR(1.0f);

			Air_P=Bottle_P=0;
			AirLevel=WaterLevel=0;
			Bottle_ref=Air_ref=													_BAR(1.0f);

			mode.Vibrate=false;
			mode.Water=false;
			mode.Air=false;
			idx=0;
			
			readyTimeout=0;
			offsetTimeout=__time__ + 5000;

			pFit=new _FIT();
			pFit->rp[0]=18726;
			pFit->rp[1]=3551*1e-4f;
			pFit->rp[2]=-1003*1e-8f;
			sim=NULL;
}
/*******************************************************************************
* Function Name				:
* Description					: 
* Output							:
* Return							: None
*******************************************************************************/
#define	_P_THRESHOLD  0x8000

_err	_SPRAY::Status() {
//------------------------------------------------------------------------------
_err	_err = adcError();
//------------------------------------------------------------------------------
			if(offsetTimeout && __time__ > offsetTimeout) {
				offset.bottle += fval.air - offset.air;
				offset.air = fval.air;
				offsetTimeout=0;
			}
//------------------------------------------------------------------------------
			Air_ref			= offset.air + AirLevel*gain.air/10;
			Bottle_ref	= offset.bottle + (Air_ref - offset.air)*pFit->Eval(Air_ref - offset.air)/0x10000 + gain.bottle*WaterLevel/10;
//
//			Bottle_ref	= offset.bottle + (Air_ref - offset.air)*waterGain/0x10000 + gain.bottle*WaterLevel/10;
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
					bottle_event=-1;
				}
				if(Bottle_P > _P_THRESHOLD) {
					Bottle_P=0;
					BottleIn->Open(120);
					BottleOut->Close();
					if(readyTimeout)
						readyTimeout = __time__ + _SPRAY_READY_T;
					bottle_event=1;
				}
			} else {
					BottleIn->Close();
					BottleOut->Open();
			}

			if((fval.compressor-offset.compressor)/gain.compressor == 0)
				_err = _err | _sprayInPressure;		
			if(readyTimeout && __time__ < readyTimeout)
				_err = _err | _sprayNotReady;
			else
				readyTimeout=0;

			if(mode.Water)
				Water->Open();
			else
				Water->Close();	

			if(AirLevel && mode.Air) {
				if(Air_ref > (int)fval.air)
					Air_P=std::min(__PWMRATE, ++Air_P);
				if(Air_ref < (int)fval.air)
					Air_P=std::max(__PWMRATE/10, --Air_P);
				
				if(mode.Vibrate && __time__ % 30 > 10)
					Air->Close();
				else
					Air->Set(Air_P);					
			}	else {
				Air->Close();
			}

			if(sim)
				sim->Poll(_IOC::parent);

			return _err;
}
//_________________________________________________________________________________
void	_SPRAY::Newline(void) {
			int i=100*(fval.air-offset.air)/_BAR(1);
			int j=100*(fval.bottle-offset.bottle)/_BAR(1);
//			int k=mode.Simulator ? 100*Pext-100 :  100*(fval.compressor-offset.compressor)/_BAR(1);
			int k=100*(fval.compressor-offset.compressor)/_BAR(1);

			if(!mode.Setup) {
				_print("\r:spray %3d,%5d,%2d.%02d,%2d.%02d,%2d.%02d",
					AirLevel,WaterLevel, i/100, abs(i%100), j/100, abs(j%100), k/100, abs(k%100));

				if(mode.Air) 
					_print("   Air"); 
				else 
					_print("   ---"); 
				if(mode.Water) 
					_print(" Water"); 
				else 
					_print("   ---"); 
				for(int i=1+6*(6-idx); i--; _print("\b"));		
			}
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
					case '-':
						Increment(-1,0);
					break;
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
						pFit->rp[0]=std::min((int)pFit->rp[0]+1000,_BAR(3.0f));
						break;
					case __PageDown:
						pFit->rp[0]=std::max((int)pFit->rp[0]-1000,_BAR(0.1f));
						break;					
					case __CtrlV:
						if(mode.Vibrate)
							mode.Vibrate=false;
						else
							mode.Vibrate=true;
						break;
						
//					case __Delete:
//						if(AirLevel || WaterLevel)
//							_print("\r\n: Air/Water not 0.... \r\n:");
//						else {
//							_ADC::offset.air = _ADC::fval.air;
//							_ADC::offset.bottle = _ADC::fval.bottle;
//							_print("\r\n: air/water offset.... \r\n:");
//							pFit=new _FIT();
//							pFit->rp[0]=_BAR(1.2f);
//						}
//						break;
//					case __End:
//						if(pFit && pFit->Compute()) {
//							_print("\r\n: fit computed  .... \r\n:");
//							break;
//						}
//						_print("\r\n: error, not enough samples .. \r\n:");
//						break;
//					case __Insert:
//						_print("\r\n: samples added %d.... \r\n:",
//							pFit->Sample(Air_ref - offset.air, pFit->rp[0]));
//						break;
						
					case __CtrlS:
						if(sim) {
							delete sim;
							sim=NULL;
							new _ADC;
							printf("\r\n: simulator deactivated...\r\n:");
						} else {
							HAL_ADC_DeInit(&hadc1);
							sim = new _SIMULATOR;
							offsetTimeout=__time__+5000;
							printf("\r\n: simulator active     ...\r\n:");
						}
						break;
					case __F1:
					case __f1:
					case __Delete:
						Adjust();
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
int		i,j,k;
			f_gets(c,sizeof(c),f);
			sscanf(c,"%hu,%hu,%hu,%hu",&offset.cooler,&offset.bottle,&offset.compressor,&offset.air);
			f_gets(c,sizeof(c),f);
			sscanf(c,"%hu,%hu,%hu,%hu",&gain.cooler,&gain.bottle,&gain.compressor,&gain.air);
			f_gets(c,sizeof(c),f);
			if(sscanf(c,"%d,%d,%d",&i,&j,&k)==3) {
				pFit->rp[0]=i;
				pFit->rp[1]=(float)j*1e-4f;
				pFit->rp[2]=(float)k*1e-8f;
			}
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
int		a=pFit->rp[0];
int		b=pFit->rp[1]*1e4f;
int		c=pFit->rp[2]*1e8f;
			f_printf(f,"%5d,%5d,%5d                       /.. pressure fit coeff.\r\n",a,b,c);
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
					if(a)
						readyTimeout	= __time__ + _SPRAY_READY_T;
					break;
				case 1:
					WaterLevel 		= std::min(std::max(0,WaterLevel+a),10);
					if(a)
						readyTimeout	= __time__ + _SPRAY_READY_T;
					break;
						case 2:
							break;
				case 3:
					gain.bottle		= std::min(std::max(_BAR(0.1f),gain.bottle+100*a),_BAR(0.5f));
					break;
				case 4:
//					if(mode.Simulator) {
//						Pext 				= std::min(std::max(0.5f,Pext+(float)a/10.0f),1.5f);
//						if(a) {
//							AirLevel = WaterLevel;
//							mode.Air = mode.Water = false;
//							offsetTimeout = __time__ + 3000;
//						}
//					}
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
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
void	*_SPRAY::Adjust() {
	int kbhit=getchar();
			_FIT *f=pFit;
			pFit=new _FIT();
			pFit->rp[0]=_BAR(1.2f);
						
			AirLevel = WaterLevel = bottle_event = 0;
			mode.Air=mode.Water=false;
			_print("\r\n: continue to spray setup (y/n)");

			while(kbhit != 'y' && kbhit != 'n') {
				_wait(100);
				kbhit=getchar();				
			}
			
			if(kbhit=='y')
				kbhit=EOF;

			for(AirLevel = 1; AirLevel<=10 && kbhit == EOF; ++AirLevel) {
				mode.Air=mode.Water=false;
				bottle_event=0;
				_print("\r\n: initializing, level %d ",AirLevel);
				for(int t=__time__+5000, k=0; __time__ < t && kbhit == EOF; _wait(2)) {
					if(bottle_event) {
						if(bottle_event == k)
							t=__time__+10000;
						_print(".");
						k=bottle_event;
						bottle_event=0;
					}
					kbhit=getchar();
				}
				
				if(kbhit == EOF) {
					mode.Air=mode.Water=true;
					_print("\r\n: backpressure adjust ",AirLevel);
					_wait(3000);
				}
				for(int t=__time__+10000, k=0; __time__ < t && kbhit == EOF; _wait(2)) {
					if(bottle_event) {
						pFit->rp[0]=std::max(std::min((int)pFit->rp[0]-1000*bottle_event,_BAR(3.0f)),_BAR(0.1f));
						if(bottle_event == k)
							t=__time__+10000;
						_print(".");
						k=bottle_event;
						bottle_event=0;
					}
					kbhit=getchar();
				}
				pFit->Sample(Air_ref - offset.air, pFit->rp[0]);
			}
			
			if(kbhit == EOF && pFit && pFit->Compute()) {
				_print("\r\n: finished\r\n");			
				delete f;
			} else {
				_print("\r\n: aborted\r\n");
				delete pFit;
				pFit=f;
			}
			AirLevel=WaterLevel=0;
			mode.Air=mode.Water=false;
			return NULL;
}
