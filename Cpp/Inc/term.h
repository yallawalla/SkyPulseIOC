#ifndef TERM_H
#define TERM_H
#include "io.h"
#include "ascii.h"
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
class _TERM {
	private:
		struct {	
			uint32_t	seq;
			uint32_t	timeout;
		} esc, rpt;
		char 		*cmdbuf,*cmdp,error;
	public:
		_TERM() {
//			io=NULL;
			esc.seq=esc.timeout=0;
			rpt.seq=rpt.timeout=0;
			cmdp=cmdbuf=new char[__CMDLEN];
		};
//	_io	*io;
	bool	Cmd(int c);
	char	*Cmd(void);
	int		Escape(void);
	void	Repeat(int,int);
	void *Parse(void);
	void *Parse(int);
	void *Parse(_io *);
	void *Parse(FIL *);
		
	virtual	void		Newline(void)			{ _print("\r\n>");	}
	virtual int			Fkey(int fkey)		{	return fkey;			};
	virtual FRESULT	Decode(char *)		{	return FR_OK;			};
};

#endif
