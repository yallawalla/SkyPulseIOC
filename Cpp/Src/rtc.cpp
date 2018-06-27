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
#include	"rtc.h"
#include	"misc.h"
#include 	"proc.h"
#include	<string>

using namespace std;
/*******************************************************************************
* Function Name				:
* Description					: 
* Output							:
* Return							: None
*******************************************************************************/
_RTC::_RTC() {	
	idx=0;
}
/*******************************************************************************
* Function Name				:
* Description					: 
* Output							:
* Return							: None
*******************************************************************************/
_RTC::~_RTC() {	
}
//_________________________________________________________________________________
void	_RTC::Newline(void) {
			io=_stdio(NULL);
			_stdio(io);
			if(io) {
				HAL_RTC_GetTime(&hrtc,&time,RTC_FORMAT_BIN);
				HAL_RTC_GetDate(&hrtc,&date,RTC_FORMAT_BIN);
				_print("\r:time        %4s,%3d-%3s-%3d,%3d::%02d::%02d",
					Days[date.WeekDay-1],date.Date,Months[date.Month-1],date.Year,
						time.Hours,time.Minutes,time.Seconds);
				for(int i=1+4*(6-idx); i--; _print("\b"));
			}
			Repeat(1000,__CtrlR);				
}
//_________________________________________________________________________________
int		_RTC::Fkey(int t) {
			switch(t) {
				case __f9:
				case __F9:
					io=_stdio(NULL);
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
void	_RTC::Increment(int a, int b) {
			idx= std::min(std::max(idx+b,0),6);
			switch(idx) {
				case 0:
					date.WeekDay+=a;
					break;
				case 1:
					date.Date+=a;
					break;
				case 2:
					date.Month+=a;
					break;
				case 3:
					date.Year+=a;
					break;
				case 4:
					time.Hours+=a;
					break;
				case 5:
					time.Minutes+=a;
					break;
				case 6:
					time.Seconds+=a;
					break;
			}			
			if(a) {
				HAL_RTC_SetTime(&hrtc,&time,RTC_FORMAT_BIN);
				HAL_RTC_SetDate(&hrtc,&date,RTC_FORMAT_BIN);
			}
			Newline();
}		
