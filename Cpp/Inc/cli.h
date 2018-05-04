#ifndef CLI_H
#define CLI_H

#include "ff.h"
#include "term.h"
#include "misc.h"
#include "proc.h"

class _FS {
	public:	
	static	FATFS fatfs;
	static	DIR		dir;			
	static	TCHAR lfn[_MAX_LFN + 1];
	static	FILINFO	fno;
	
	_FS() {
			if(f_getcwd(lfn,_MAX_LFN) != FR_OK) {
				f_mount(&fatfs,"0:",1);
				f_opendir(&dir,"/");
			}
	}
};
//_________________________________________________________________________________
class _CLI : public _TERM, public _FS {
	private:
		virtual FRESULT	DecodePlus(char *),
										DecodeMinus(char *),
										DecodeInq(char *),
										DecodeEq(char *);
	public:	
		_io			*io;
		virtual void		Newline(void);
		virtual FRESULT	Decode(char *);
		virtual int			Fkey(int);

		static void			parseTask(_CLI *me) {
			me->Parse(me->io);
		}

		_CLI()	{
			io=ioUsart(NULL,__RXLEN,__TXLEN);
			_proc_add((void *)parseTask,this,(char *)"Cli",0);
		};

		_CLI(UART_HandleTypeDef *huart)	{
			io=ioUsart(huart,__RXLEN,__TXLEN);
			_proc_add((void *)parseTask,this,(char *)"Usart Cli",0);
		};
		
		_CLI(USBD_HandleTypeDef *usbd) {
			_proc_add((void *)parseTask,this,(char *)"Usb Cli",0);
			_proc_add((void *)CDC_Poll_FS,&io,(char *)"Tx VCP",0);
		};

		~_CLI(void)	{};
};
#endif
