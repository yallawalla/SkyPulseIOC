#ifndef TERM_H
#define TERM_H
#include "io.h"
#include "fsw.h"
#include "ascii.h"
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
typedef enum {
	DBG_OFF						=0,
	DBG_CAN_TX				=1<<0,
	DBG_CAN_RX				=1<<1,
	DBG_ERR						=1<<2,
	DBG_INFO					=1<<3,
	DBG_COM_CAN				=1<<21,
	DBG_EC_SIM				=1<<22,
	DBG_ENRG					=1<<23
}	_dbg;
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
			esc.seq=esc.timeout=0;
			rpt.seq=rpt.timeout=0;
			cmdp=cmdbuf=new char[__CMDLEN];
		};

static	_dbg	debug;
static	_io		*dbgio;
static	void	Debug(_dbg, const char *format, ...);

	bool	Cmd(int c);
	char	*Cmd(void);
	int		Escape(void);
	void	Repeat(int,int);
	void *Parse(void);
	void *Parse(int);
	void *Parse(_io *);
	void *Parse(FIL *);

	virtual	FRESULT	Batch(char *);		
	virtual FRESULT	Decode(char *)		{	return FR_OK;			};
	virtual	void		Newline(void)			{ _print("\r\n>");	}
	virtual int			Fkey(int fkey)		{	return fkey;			};
};
#endif
