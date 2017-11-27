#include "term.h"
/**
******************************************************************************
* @file
* @author  Fotona d.d.
* @version
* @date
* @brief	 
*
*/
/** @addtogroup
* @{
*/
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void	_TERM::Repeat(int t) {
			timeout = -(HAL_GetTick() + t);
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
char	*_TERM::Cmd(void) {
			return cmdp;
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
bool	_TERM::Cmd(int c) {
			switch(c) {
				case EOF:
					break;
				case __BACKSPACE:
				case __DELETE:
					if(cmdp != cmdbuf) {
						--cmdp;
					__print("\b \b");
					}
					break;

				case __LF:
					break;
				case __CR:
					*cmdp=0;
					cmdp=cmdbuf;		
					return true;

				default:
					if(c < ' ' || c > 127)
						__print("<%02X>",c);
					else {
						__print("%c",c);
						*cmdp++=c;
					}
				}
			return false;
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
int		_TERM::Escape(void) {
int		i=getchar();

			if(i==EOF) {
				if(timeout && (HAL_GetTick() > abs(timeout))) {
					timeout=0;
					return seq;
					}
			} else if(timeout > 0) {
				seq=(seq<<8) | i;
				if(i=='~' || i=='A' || i=='B' || i=='C' || i=='D') {
					timeout=0;
					return seq;
				}
			} else if(i==__Esc) {
				timeout=HAL_GetTick()+10;
				seq=i;
			} else {
				timeout=0;
				return i;
			}
			return EOF;
}
//______________________________________________________________________________________
void	*_TERM::Parse(_io *io) {
_io		*temp=_stdio(io);
void	*v=Parse();
			_stdio(temp);
			return v;
}
//______________________________________________________________________________________
void	*_TERM::Parse(FIL *f) {
			return Parse(fgetc((FILE *)f));
}
//______________________________________________________________________________________
void	*_TERM::Parse(void) {

	return Parse(Escape());
}
//______________________________________________________________________________________
void	*_TERM::Parse(int i) {
void 	*v=this;
			switch(i) {
				case EOF:
					break;
				case __CtrlZ:
					while(1);
				case __CtrlY:
					NVIC_SystemReset();
				break;
				default:
					i=Fkey(i);
					if(i==EOF)
						break;
					if(i==__f12 || i==__F12) {
						v=NULL;
						i=__CR;
					}
					if(Cmd(i)) {
						error=Decode(Cmd());
						if(error != 0)
							__print("... WTF(%d)",error);
						Newline();
					}
			}
		return v;
}
/**
* @}
*/
