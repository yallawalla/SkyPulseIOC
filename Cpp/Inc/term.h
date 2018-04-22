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
	typedef enum {
	__FSW_OFF	=((FSW3_Pin | FSW2_Pin | FSW0_Pin)	<<8),
	__FSW_1		=((											 FSW0_Pin)	<<8),
	__FSW_2		=((FSW3_Pin 					 | FSW0_Pin)	<<8),
	__FSW_3		=((FSW3_Pin 										 )	<<8),
	__FSW_4		=((FSW3_Pin | FSW2_Pin 					 )	<<8)
} __FOOT;
	
#define __FSW (((FSW0_GPIO_Port->IDR & FSW0_Pin) |  \
					(FSW2_GPIO_Port->IDR & FSW2_Pin) | 				\
						(FSW3_GPIO_Port->IDR & FSW3_Pin))<<8)
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
		struct {
			int key,
					temp,
					timeout;
		} fsw;
	public:
		_TERM() {
			fsw.timeout=0;
			fsw.key=__FSW;
			esc.seq=esc.timeout=0;
			rpt.seq=rpt.timeout=0;
			cmdp=cmdbuf=new char[__CMDLEN];
		};

	bool	Cmd(int c);
	char	*Cmd(void);
	int		Escape(void);
	int		Fsw(void);
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
