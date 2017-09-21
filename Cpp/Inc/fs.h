#ifndef FS_H
#define FS_H

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
class _FS:public _TERM, public _FAT {
	private:
		int	wcard(char *, char *);
		int	find_recurse (char *, char *, int);

	public:	
		virtual void		Newline(void);
		virtual FRESULT	Decode(char *);
		virtual int			Fkey(int);
	
		static void			pollUart(_FS *me) {
			me->Parse();
		}
		
		static void			pollUsb(_FS *me) {
			me->io=_VCP;
			me->Parse();
		}
	
		_FS(UART_HandleTypeDef *huart)	{
			if(f_getcwd(lfn,_MAX_LFN) != FR_OK) {
				f_mount(&fatfs,"0:",1);
				f_opendir(&dir,"/");
			}
			if(huart)
				io=init_uart(huart,__RXLEN,__TXLEN);
			else
				io=_io_init(__RXLEN,__TXLEN);
			_proc_add((void *)pollUart,this,(char *)"pollUart",0);
		};
		
		_FS()	{
			if(f_getcwd(lfn,_MAX_LFN) != FR_OK) {
				f_mount(&fatfs,"0:",1);
				f_opendir(&dir,"/");
			}
			_proc_add((void *)pollUsb,this,(char *)"FS",0);
		};

		~_FS(void)	{};
};

#endif
