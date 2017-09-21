#ifndef TERM_H
#define TERM_H
#include "stdio.h"
#include "ascii.h"
#include "io.h"
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
class _TERM {
	private:
		int			seq, timeout,error;
		char 		*cmdbuf,*cmdp;
	public:
		_TERM() {
			io=NULL;
			seq=timeout=0;
			cmdp=cmdbuf=new char[__CMDLEN];
		};
	_io	*io;
	bool	Cmd(int c);
	char	*Cmd(void);
	int		Escape(void);
	void	Repeat(int);
	void *Parse(void);
		
	virtual void	Newline(void);
	virtual int		Fkey(int fkey)				{	return fkey;			};
	virtual FRESULT	Decode(char *)			{	return FR_OK;			};
};

#endif
