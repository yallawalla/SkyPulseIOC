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
#include	"time.h"
#include	"misc.h"
#include	<string>
using namespace std;
/*******************************************************************************
* Function Name				:
* Description					: 
* Output							:
* Return							: None
*******************************************************************************/
_TIME::_TIME() {	
	idx=0;
}
//_________________________________________________________________________________
string Days[]={"Mon","Tue","Wed","Thu","Fri","Sat","Sun"};
string Months[]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
void _TIME::Newline(void) {
RTC_TimeTypeDef t;
RTC_DateTypeDef d;
		HAL_RTC_GetTime(&hrtc,&t,RTC_FORMAT_BIN);
		HAL_RTC_GetDate(&hrtc,&d,RTC_FORMAT_BIN);
	printf("\r:time        %4s,%3d-%3s-%3d,%3d::%02d::%02d",
			Days[d.WeekDay-1].c_str(),d.Date,Months[d.Month-1].c_str(),d.Year,t.Hours,t.Minutes,t.Seconds);
		for(int i=1+4*(6-idx); i--; printf("\b"));
}
//_________________________________________________________________________________
int		_TIME::Fkey(int t) {
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
void	_TIME::Increment(int a, int b) {
			idx= std::min(std::max(idx+b,0),6);
			switch(idx) {
				case 0:
					break;
				case 1:
					break;
				case 2:
					break;
				case 3:
					break;
			}
			Newline();
}		
