#include "term.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "io.h"
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
void	_TERM::Newline(void) {
			printf("\r\n>");
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
					printf("\b \b");
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
						printf("<%02X>",c);
					else {
						printf("%c",c);
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
				timeout=HAL_GetTick()+5;
				seq=i;
			} else {
				timeout=0;
				return i;
			}
			return EOF;
}
//______________________________________________________________________________________
void	*_TERM::Parse(void) {
void 	*v=this;
_io			*temp=_stdio(io);
int			i;
				do {
					i=Escape();
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
									printf("... WTF(%d)",error);
								Newline();
							}
					}
				} while(i != EOF);
				_stdio(temp);
			return v;
}

/**
* @}
*/
