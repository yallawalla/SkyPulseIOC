#ifndef CLI_H
#define CLI_H

#include "ff.h"
#include "term.h"
#include "misc.h"
#include "proc.h"

class _FAT {
	public:	
	static	FATFS fatfs;
	static	DIR		dir;			
	static	TCHAR lfn[_MAX_LFN + 1];
	static	FILINFO	fno;
};
//_________________________________________________________________________________
class _CLI : public _TERM, public _FAT {
	private:
		FRESULT	DecodePlus(char *),DecodeMinus(char *),DecodeInq(char *),DecodeEq(char *);
		int			wcard(char *, char *);
		int			find_recurse (char *, char *, int);
		void		printRtc(void);

	public:	
		_io			*io;
		virtual void		Newline(void);
		virtual FRESULT	Decode(char *);
		virtual int			Fkey(int);

		static void			parseUsart(_CLI *me) {
			me->Parse(me->io);
		}
		static void			parseUsb(_CLI *me) {
			me->io = _VCP;
			me->Parse(me->io);
		}

		_CLI(UART_HandleTypeDef *huart)	{
			if(f_getcwd(lfn,_MAX_LFN) != FR_OK) {
				f_mount(&fatfs,"0:",1);
				f_opendir(&dir,"/");
			}
			io=init_uart(huart,__RXLEN,__TXLEN);
			_proc_add((void *)parseUsart,this,(char *)"Usart Cli",0);
		};
		
		_CLI() {
			if(f_getcwd(lfn,_MAX_LFN) != FR_OK) {
				f_mount(&fatfs,"0:",1);
				f_opendir(&dir,"/");
			}
			_proc_add((void *)parseUsb,this,(char *)"Usb Cli",0);
			_proc_add((void *)CDC_Poll_FS,NULL,(char *)"Tx VCP",0);
		};

		~_CLI(void)	{};
};
#endif
