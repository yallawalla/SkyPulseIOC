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
_io*	_TERM::dbgio=NULL;
_dbg	_TERM::debug=DBG_OFF;
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
				case __CR:
					*cmdp=0;
					cmdp=cmdbuf;		
					return true;

				default:
					if(c >= ' ' && c < 128) {
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
FRESULT _TERM::Batch(char *filename) {
			FIL *f=new FIL;
			FRESULT ret=f_open(f,filename,FA_READ);
			if(ret==FR_OK) {
				while(!f_eof(f))
					Parse(f);
				f_close(f);	
			}
			delete f;
			return ret;
}
/*******************************************************************************
* Function Name : batch
* Description   :	ADP1047 output voltage setup, using the default format
* Input         :
* Output        :
* Return        :
*******************************************************************************/
extern "C" {int		printk_va(char **, const char *, va_list);}
void	_TERM::Debug(_dbg d, const char *format, ...) {
			if(debug & (d)) {
				_io *temp=_stdio(dbgio);
				va_list	aptr;
				va_start(aptr, format);
				printk_va(0, format, aptr );
				va_end(aptr);
				_stdio(temp);
			}
}
/**
* @}
*/
