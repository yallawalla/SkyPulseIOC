#include "term.h"
#include "misc.h"
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
void	_TERM::Repeat(int t, int ch) {
			rpt.timeout = __time__ + t;
			rpt.seq=ch;
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
					_print("\b \b");
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
						_print("<%02X>",c);
					else {
						_print("%c",c);
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
				if(esc.timeout && (__time__ > esc.timeout)) {
					esc.timeout=0;
					return esc.seq;
					}
				if(rpt.timeout && (__time__ > rpt.timeout)) {
					rpt.timeout=0;
					return rpt.seq;
					}
			} else if(esc.timeout > 0) {
				esc.seq=(esc.seq<<8) | i;
				if(i=='~' || i=='A' || i=='B' || i=='C' || i=='D') {
					esc.timeout=0;
					return esc.seq;
				}
			} else if(i==__Esc) {
				esc.timeout=__time__+10;
				esc.seq=i;
			} else {
				esc.timeout=0;
				return i;
			}
			return Fsw();
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
_io		*temp=_stdio(NULL);
void	*v=Parse(fgetc((FILE *)f));
			_stdio(temp);
			return v;
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
							_print("... WTF(%d)",error);
						Newline();
					}
			}
		return v;
}

/*******************************************************************************
* Function Name	:
* Description		:
* Output				:
* Return				:
*******************************************************************************/
int		_TERM::Fsw() {	
			int i=__FSW;
			if(i != fsw.temp) {
				fsw.temp = i;
				fsw.timeout = __time__ + 10;
			} else 
					if(fsw.timeout && __time__ > fsw.timeout) {
						fsw.timeout=0;
						if(fsw.temp != fsw.key ) {
							fsw.key=fsw.temp ;
							return (fsw.key);
						}
			}
			return EOF;
}
/**
* @}
*/
