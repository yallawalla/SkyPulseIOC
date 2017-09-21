#include "term.h"
#include <ctype.h>
#include <string>
#include "fs.h"
#include "can.h"
#include "misc.h"
#include "proc.h"
#include <string.h>

FATFS		_FAT::fatfs;
DIR			_FAT::dir;			
TCHAR		_FAT::lfn[_MAX_LFN + 1];
FILINFO	_FAT::fno;
//_________________________________________________________________________________
void _FS::Newline(void) {
	printf("\r\n");
	if(f_getcwd(lfn,_MAX_LFN)==FR_OK && f_opendir(&dir,lfn)==FR_OK) {
		if(lfn[strlen(lfn)-1]=='/')
			printf("%s",lfn);
				else
					printf("%s/",lfn);
	} else
	printf("?:/"); 		
}
//_________________________________________________________________________________
int	_FS::Fkey(int t) {
		switch(t) {
			case __f12:
			case __F12:
				_CAN	*c=_CAN::InstanceOf(&hcan2);
				c->Newline();
				while(c->Parse())
					_wait(2,_proc_loop);
				break;
		}
		return t;
}
//_________________________________________________________________________________
typedef enum  { _LIST, _ERASE } _FACT;
//_________________________________________________________________________________
FRESULT _FS::Decode(char *p) {
	char *sc[]={0,0,0,0,0,0,0,0};
	int i=0,n=0,len=1;

	while (p[i]) {
		while(p[i] && p[i]==' ')
			p[i++]=0;
		if(p[i])
			sc[n++]=&p[i];
		while(p[i]!=' ' && p[i])
			++i;
	}
	if(!sc[0])
		return FR_OK;
	len=strlen(sc[0]);
//_________________________________________________________________________________
	if(!(strncmp("0:",sc[0],len) && strncmp("1:",sc[0],len))) {
		if(FRESULT err=f_mount(&fatfs,sc[0],1))
			return err;
		if(FRESULT err=f_chdrive(sc[0]))
			return err;
		if(FRESULT err=f_getcwd(lfn,_MAX_LFN))
			return err;
		if(FRESULT err=f_opendir(&dir,lfn))
			return err;
	}
//__change directory_______________________________________________________________
	else if(!strncmp("cdir",sc[0],len)) {
		if(n < 2)
			return FR_NO_FILE;
		if(FRESULT err=f_chdir(sc[1]))
			return err;
	}
//__change directory_______________________________________________________________
	else if(!strncmp("eject",sc[0],len)) {
		if(n < 2)
			return FR_DISK_ERR;
		if(FRESULT err=f_mount(NULL,sc[0],1))
			return err;
	}
//__list directory_________________________________________________________________
	else if(!strncmp("directory",sc[0],len)) {
		if(n==1)
			sc[1]=(char *)"*";
		if(FRESULT err=f_readdir(&dir,NULL))
			return err;	
		do {
			if(FRESULT err=f_readdir(&dir,&fno))
				return err;	
			if(dir.sect) {
				char *p=fno.fname;
					p=fno.fname;
				if(wcard(sc[1],p)) {
					printf("\r\n%-16s",p);
					if (fno.fattrib & AM_DIR)
						printf("/");
					else
						printf("%d",(int)fno.fsize);								
				}
			}
		} while(dir.sect);
	}
//__delete file____________________________________________________________________
	else if(!strncmp("ls",sc[0],len))
		find_recurse(lfn,sc[1],_LIST);
//__delete file____________________________________________________________________
	else if(!strncmp("er",sc[0],len))
		find_recurse(lfn,sc[1],_ERASE);
//__delete file____________________________________________________________________
	else if(!strncmp("delete",sc[0],len)) {
		if(n==1)
		sc[1]=(char *)"*";
		do {
			if(FRESULT err=f_readdir(&dir,&fno))
				return err;	
			if(dir.sect) {
				char *p=fno.fname;
				if(wcard(sc[1],p)) {
					f_unlink(p);			
				if(FRESULT err=f_unlink(p))
					return err;	
				}
			}
		} while(dir.sect);
	}
//__rename file____________________________________________________________________
	else if(!strncmp("rename",sc[0],len)) {
		if(n < 3)
			return FR_NO_FILE;
		if(FRESULT err=f_rename(sc[1],sc[2]))
			return err;	
	}
//__type file______________________________________________________________________
	else if(!strncmp("type",sc[0],len)) {
		if(n < 2)

		return FR_NO_FILE;
		else {
			FIL	f;
			if(FRESULT err=f_open(&f,sc[1],FA_READ))
				return err;	
			printf("\r\n");
			while(!f_eof(&f)) 
				printf("%c",f_getc(&f));
			f_close(&f);
		}
	}
//__make directory_________________________________________________________________
	else if(!strncmp("mkdir",sc[0],len)) {
		if(n < 2)
			return FR_NO_FILE;
		if(FRESULT err=f_mkdir(sc[1]))
			return err;	
	}
//__copy file______________________________________________________________________
	else if(!strncmp("copy",sc[0],len)) {
		char f[256];
		FIL f1,f2;
		if(n == 2) {
			p=strchr(sc[1],':');
			if(p++) {
				if(*p=='/')
					++p;
				strcpy(f,p);
			} else
				strcpy(f,sc[1]);
		}
		else
		if(n == 3) {
			strcpy(f,sc[2]);	
		} else
			return FR_NO_FILE;
		
		if(!strcmp(sc[1],f))
			strcat(f,"_Copy");
	
		if(f[strlen(f)-1]==':')
			strcat(f,sc[1]);
		if(FRESULT err=f_open(&f1,sc[1],FA_READ))
			return err;	
		if(FRESULT err=f_open(&f2,f,FA_CREATE_ALWAYS | FA_WRITE)) {
			f_close(&f1);
			return err;	
		}
		while(!f_eof(&f1))
			if(fputc(fgetc((FILE *)&f1),(FILE *)&f2)==EOF)
				break;
		if(!f_eof(&f1)) {
			f_close(&f1);
			f_close(&f2);
			return FR_NO_FILE;
		}
		f_close(&f1);
		f_close(&f2);
	}
////__entering new file____________________________________________________________
//						if(!strncmp("file",sc[0],len)) {
//							if(n == 2)
//								return(EnterFile(sc[1]));
//							else
//								return _PARSE_ERR_SYNTAX;
//						}
//__entering new file______________________________________________________________
	else if(!strncmp("format",sc[0],len)) {
		if(n < 2)
			return FR_NO_FILE;
		if(!strncmp("0:",sc[1],len)) {
			FLASH_Erase(FLASH_SECTOR_6,2);printf(".");_wait(10,_proc_loop);
			FLASH_Erase(FLASH_SECTOR_8,2);printf(".");;_wait(10,_proc_loop);
			FLASH_Erase(FLASH_SECTOR_10,2);printf(".\r\n");;_wait(10,_proc_loop);
		}		
		FRESULT err=f_mount(&fatfs,sc[1],1);
		if(FRESULT err=f_mkfs(sc[1],0,CLUSTER_SIZE))
			return err;	
	}
//__dump memory contents___________________________________________________________
	else if(!strncmp("dump",sc[0],len)) {
		dumpHex(strtoul( sc[1],NULL,0),strtoul( sc[2],NULL,0));
	}
//__dump memory contents___________________________________________________________
	else if(!strncmp("file",sc[0],len)) {
		class _ENTERFILE : public _TERM, public _FAT {
			private:
				FIL f;
			public:
				FRESULT err;
				_ENTERFILE(char *filename) {
					err=f_open(&f,filename,FA_CREATE_ALWAYS | FA_WRITE);
					Newline();
				};
				~_ENTERFILE(void) {
					f_close(&f);
				};
				virtual FRESULT Decode(char *c) {
					f_printf(&f,"%s\r\n",c);
					return FR_OK;
				};
				virtual void Newline(void) {
					printf("\r\n");
				};	
		} efile(sc[1]);
		
		while(efile.Parse() && efile.err == FR_OK) {
			_wait(2,_proc_loop);
		}
		return efile.err;
	}
//_________________________________________________________________________________
//	else if(!strncmp("?lcclkl<?",sc[0],len)) {
//		char *c,fs[256];
//		FIL f1,f2;
//	
//		if(n < 3)
//			return(_PARSE_ERR_MISSING);
//		if(f_open(&f1,sc[1],FA_READ)!=FR_OK)
//			return _PARSE_ERR_OPENFILE;
//		if(f_open(&f2,sc[2],FA_WRITE | FA_OPEN_ALWAYS)!=FR_OK) {
//			f_close(&f1);
//			return _PARSE_ERR_OPENFILE;
//		};
//		
//		while(fgets(fs,sizeof(fs),(FILE *)&f1))
//			for(c=fs;c < fs + strlen(fs)-2; f_putc(strtol(c,&c,16),&f2));
//		
//		f_close(&f1);
//		f_close(&f2);
//		return _PARSE_OK;						
//	}
//__entering new file______________________________________________________________
	else if(!strncmp("usb",sc[0],len)) {
//		if(n < 2) 
//			return _PARSE_ERR_MISSING;
//		if(!strncmp("host",sc[1],len)) {
//			USBH_Init(&USBH_Device, USBH_UserProcess, 0);
//			USBH_RegisterClass(&USBH_Device, USBH_MSC_CLASS);
//			USBH_Start(&USBH_Device);
//			_proc_add((void *)USBH_Process,&USBH_Device,(char *)"usb host",0);
//		} else if(!strncmp("filesystem",sc[1],strlen(sc[1]))) {
//			USBD_Init(&USBD_Device, &MSC_Desc, 0);
//			USBD_RegisterClass(&USBD_Device, USBD_MSC_CLASS);
//			USBD_MSC_RegisterStorage(&USBD_Device, &USBD_DISK_fops);
//			USBD_Start(&USBD_Device);			
//		} else if(!strncmp("serial",sc[1],strlen(sc[1]))) {
//			USBD_Init(&USBD_Device, &VCP_Desc, 0);
//			USBD_RegisterClass(&USBD_Device, USBD_CDC_CLASS);
//			USBD_CDC_RegisterInterface(&USBD_Device, &USBD_CDC_fops);
//			USBD_Start(&USBD_Device);
//		} else
				return FR_NOT_READY;
	} else {
		if(n) {
			for(i=0; i<n; ++i)
				printf(" %s",sc[i]);
			return FR_INVALID_NAME;
		}
	}
	return FR_OK;
}
//_________________________________________________________________________________
int	_FS::wcard(char *t, char *s) {
			return *t-'*' ? *s ? (*t=='?') | (toupper(*s)==toupper(*t)) && wcard(t+1,s+1) : 
				!*t : 
					wcard(t+1,s) || (*s && wcard(t,s+1));
}
//_________________________________________________________________________________
int	_FS::find_recurse (char * dir_name, char *w, int fact) {
DIR	dir;
FILINFO	fno;
		if (f_opendir(&dir,dir_name) != FR_OK)
			return (EXIT_FAILURE);
		while (1) {
			f_readdir(&dir,&fno);
			if (!dir.sect)
				break;
			else {
				char *p=fno.fname;
					if (!strcmp (p, "..") || !strcmp (p, "."))
					continue;
				if (snprintf (lfn, sizeof(lfn), "%s/%s", dir_name, p) >= sizeof(lfn))
					return  (EXIT_FAILURE);	
				if (fno.fattrib & AM_DIR)
						find_recurse (lfn,w,fact);
				switch(fact) {
					case _LIST:
						if(wcard(w,p)) {
							char *q=strchr(dir_name,'/');
							++q;
							printf("\r\n%s",lfn);
							if (fno.fattrib & AM_DIR)
								printf("/");
							else
								printf("%*d",32-strlen(lfn),(int)fno.fsize);
						}
					break;
					case _ERASE:
						if(wcard(w,p))
							f_unlink(p);
					}
			}
		}
		if (f_closedir(&dir) != FR_OK)
			return (EXIT_FAILURE);
		return FR_OK;
}
