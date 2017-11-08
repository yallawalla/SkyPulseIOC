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
	_proc_add((void *)Refresh,this,(char *)"rtc",500);
}
/*******************************************************************************
* Function Name				:
* Description					: 
* Output							:
* Return							: None
*******************************************************************************/
_RTC::~_RTC() {	
	_proc_remove((void *)Refresh,this);
}
//_________________________________________________________________________________
string Days[]		= { "Mon","Tue","Wed","Thu","Fri","Sat","Sun" };
string Months[]	= { "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec" };

void	_RTC::Newline(void) {
	_io *temp=_stdio(io);
			HAL_RTC_GetTime(&hrtc,&time,RTC_FORMAT_BIN);
			HAL_RTC_GetDate(&hrtc,&date,RTC_FORMAT_BIN);
			printf("\r:time        %4s,%3d-%3s-%3d,%3d::%02d::%02d",
				Days[date.WeekDay-1].c_str(),date.Date,Months[date.Month-1].c_str(),date.Year,
					time.Hours,time.Minutes,time.Seconds);
			for(int i=1+4*(6-idx); i--; printf("\b"));
	_stdio(io);
}
//_________________________________________________________________________________
int		_RTC::Fkey(int t) {
			switch(t) {
				case __f9:
				case __F9:
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
