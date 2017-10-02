#include "term.h"
#include "ioc.h"


FATFS		_FAT::fatfs;
DIR			_FAT::dir;			
TCHAR		_FAT::lfn[_MAX_LFN + 1];
FILINFO	_FAT::fno;
//_________________________________________________________________________________
void _CLI::Newline(void) {
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
int	_CLI::Fkey(int t) {
		switch(t) {
			case __f5:
			case __F5:
			{
				_PUMP	*p=_PUMP::InstanceOf();
				p->Newline();
				while(p->Parse())
					_wait(2,_proc_loop);
				return __F12;
			}
			case __f6:
			case __F6:
			{
				_FAN	*p=_FAN::InstanceOf();
				p->Newline();
				while(p->Parse())
					_wait(2,_proc_loop);
				return __F12;
			}
			case __f7:
			case __F7:
			{
				_SPRAY	*p=_SPRAY::InstanceOf();
				p->Newline();
				while(p->Parse())
					_wait(2,_proc_loop);
				return __F12;
			}
			case __f8:
			case __F8:
			{
				_CAN	*c=_CAN::InstanceOf(&hcan2);
				c->Newline();
				c->io=io;
				while(c->Parse())
					_wait(2,_proc_loop);
				c->io=NULL;
				return __F12;
			}
			case __f9:
			case __F9:
			{
				_TIME	t;
				t.Newline();
				while(t.Parse())
					_wait(2,_proc_loop);
				return __F12;
			}
			case __f11:
			case __F11:
			{
				FIL f;
				if(f_open(&f,"0:/lm.ini",FA_WRITE | FA_OPEN_ALWAYS) == FR_OK) {
					_PUMP::InstanceOf()->SaveSettings((FILE *)&f);
					_FAN::InstanceOf()->SaveSettings((FILE *)&f);
					_SPRAY::InstanceOf()->SaveSettings((FILE *)&f);
					printf("... saved");
					f_close(&f);
				}	else
					printf("... error settings file");		
				Newline();
				break;
			}
			case __f1:
			case __F1:
				SetTimeDate();
				break;
			default:
				return t;
		}
		return EOF;
}
//_________________________________________________________________________________
typedef enum  { _LIST, _ERASE } _FACT;
//_________________________________________________________________________________
FRESULT _CLI::Decode(char *p) {
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
						printf("%-8s","/");
					else
						printf("%-8d",(int)fno.fsize);	
					date_time(fno.fdate,fno.ftime);
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
	else if(!strncmp("time",sc[0],len)) {
		int h, m;
		if(sscanf(sc[1],"%d:%d",&h,&m)==2) {
			RTC_TimeTypeDef sTime;
			sTime.Hours = h;
			sTime.Minutes =m;
			sTime.Seconds = 0;
			sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
			sTime.StoreOperation = RTC_STOREOPERATION_RESET;
			if(HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
				return FR_NOT_READY;
			sTime.StoreOperation = RTC_STOREOPERATION_SET;
		} else
			printRtc();
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
int	_CLI::wcard(char *t, char *s) {
			return *t-'*' ? *s ? (*t=='?') | (toupper(*s)==toupper(*t)) && wcard(t+1,s+1) : 
				!*t : 
					wcard(t+1,s) || (*s && wcard(t,s+1));
}
//_________________________________________________________________________________
string days[]={"Mon","Tue","Wed","Thu","Fri","Sat","Sun"};
string months[]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
void	_CLI::printRtc() {
RTC_TimeTypeDef t;
RTC_DateTypeDef d;
		HAL_RTC_GetTime(&hrtc,&t,RTC_FORMAT_BIN);
		HAL_RTC_GetDate(&hrtc,&d,RTC_FORMAT_BIN);
		printf("%4s,%3d-%3s-%d,%3d:%02d:%02d",days[d.WeekDay-1].c_str(),d.Date,months[d.Month-1].c_str(),d.Year,t.Hours,t.Minutes,t.Seconds);
}
//_________________________________________________________________________________
int	_CLI::find_recurse (char * dir_name, char *w, int fact) {
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
