#include "term.h"
#include "ioc.h"

FATFS		_FAT::fatfs;
DIR			_FAT::dir;			
TCHAR		_FAT::lfn[_MAX_LFN + 1];
FILINFO	_FAT::fno;
//_________________________________________________________________________________
void _CLI::Newline(void) {
		_print("\r\n");
		if(f_getcwd(lfn,_MAX_LFN)==FR_OK && f_opendir(&dir,lfn)==FR_OK) {
			if(lfn[strlen(lfn)-1]=='/')
				_print("%s",lfn);
					else
						_print("%s/",lfn);
		} else
		_print("?:/"); 		
}
//_________________________________________________________________________________
int	_CLI::Fkey(int t) {
		_IOC	*ioc=_IOC::parent;
		switch(t) {
		case __CtrlE: {
			_io *temp=ioc->can.ioFsw;
			ioc->can.ioFsw=io;
			while(ioc->can.SendRemote(idFOOT2CAN) != __CtrlE) 
				_wait(2);
			ioc->can.ioFsw=temp;
			Newline();
		} break;
			case __f5:
			case __F5:
				ioc->pump.Newline();
				while(ioc->pump.Parse()) 
					_wait(2);
				return __F12;
			case __f6:
			case __F6:
				ioc->fan.Newline();
				while(ioc->fan.Parse())
					_wait(2);
				return __F12;
			case __f7:
			case __F7:
				ioc->spray.Newline();
				while(ioc->spray.Parse())
					_wait(2);
				return __F12;
			case __f8:
			case __F8:
				ioc->can.Newline();
				while(ioc->can.Parse(io))
					_wait(2);
				return __F12;
			case __f9:
			case __F9:
			{
				ioc->rtc.Newline();
				while(ioc->rtc.Parse(io))
					_wait(2);
				return __F12;
			}
			case __f10:
			case __F10:
				ioc->ws2812.Newline();
				while(ioc->ws2812.Parse())
					_wait(2);
				return __F12;
			case __f11:
			case __F11:
			{
				FIL *f=new FIL;
				if(f_open(f,"0:/lm.ini",FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) {
					ioc->pump.SaveSettings(f);
					ioc->fan.SaveSettings(f);
					ioc->spray.SaveSettings(f);
					ioc->ws2812.SaveSettings(f);
					_print("... saved");
					f_close(f);
					delete f;
				}	else
					_print("... error settings file");		
				Newline();
			}
				break;
			
			case __FSW_OFF:
				ioc->IOC_FootAck.State=_OFF;
				ioc->IOC_FootAck.Send();
				if(ioc->debug & DBG_INFO)
					_print("\r\n:\r\n:footswitch disconnected \r\n:");					
				break;
			case __FSW_1:
				ioc->IOC_FootAck.State=_1;
				ioc->IOC_FootAck.Send();
				if(ioc->debug & DBG_INFO)
					_print("\r\n:\r\n:footswitch state 1\r\n:");					
				break;
			case __FSW_2:
				ioc->IOC_FootAck.State=_2;
				ioc->IOC_FootAck.Send();
				if(ioc->debug & DBG_INFO)
					_print("\r\n:\r\n:footswitch state 2\r\n:");					
				break;
			case __FSW_3:
				ioc->IOC_FootAck.State=_3;
				ioc->IOC_FootAck.Send();
				if(ioc->debug & DBG_INFO)
					_print("\r\n:\r\n:footswitch state 3\r\n:");					
				break;
			case __FSW_4:
				ioc->IOC_FootAck.State=_4;
				ioc->IOC_FootAck.Send();
				if(ioc->debug & DBG_INFO)
					_print("\r\n:\r\n:footswitch state 4\r\n:");					
				break;
				
			default:
				return t;
		}
		return EOF;
}
//_________________________________________________________________________________
//_________________________________________________________________________________
//_________________________________________________________________________________
//_________________________________________________________________________________
//_________________________________________________________________________________
//_________________________________________________________________________________
typedef enum  { _LIST, _ERASE } _FACT;
//_________________________________________________________________________________

FRESULT _CLI::DecodePlus(char *c) {
	_IOC	*ioc=_IOC::parent;
	switch(*++c) {
		case 'D':
			
		for(c=strchr(c,' '); c && *c;)
			ioc->debug = (_dbg)(ioc->debug | (1<<strtoul(++c,&c,10)));
		ioc->dbgio=io;
		break;
		case 'E':
		for(c=strchr(c,' '); c && *c;)
			ioc->error_mask = (_err)(ioc->error_mask | (1<<strtoul(++c,&c,10)));
		break;
		case 'W':
		for(c=strchr(c,' '); c && *c;)
			ioc->warn_mask = (_err)(ioc->warn_mask | (1<<strtoul(++c,&c,10)));
		break;
		default:
			return FR_INVALID_NAME;
	}
	return FR_OK;
}
//_________________________________________________________________________________
FRESULT _CLI::DecodeMinus(char *c) {
	_IOC	*ioc=_IOC::parent;
	switch(*++c) {
		case 'D':
		for(c=strchr(c,' '); c && *c;)
			ioc->debug = (_dbg)(ioc->debug & ~(1<<strtoul(++c,&c,10)));
		ioc->dbgio=io;
		break;
		case 'E':
		for(c=strchr(c,' '); c && *c;)
			ioc->error_mask = (_err)(ioc->error_mask & ~(1<<strtoul(++c,&c,10)));
		break;
		case 'W':
		for(c=strchr(c,' '); c && *c;)
			ioc->warn_mask = (_err)(ioc->warn_mask & ~(1<<strtoul(++c,&c,10)));
		break;
		default:
			return FR_INVALID_NAME;
	}
	return FR_OK;
}
//_________________________________________________________________________________
FRESULT _CLI::DecodeEq(char *c) {
	_IOC	*ioc=_IOC::parent;
	switch(*++c) {
		case 'E':
		for(c=strchr(c,' '); c && *c;)
			ioc->SetError((_err)(1<<strtoul(++c,&c,10)));
		break;
		default:
			return FR_INVALID_NAME;
	}
	return FR_OK;
}
//_________________________________________________________________________________
FRESULT _CLI::DecodeInq(char *c) {
	_IOC	*ioc=_IOC::parent;
	switch(*++c) {
		case 'E':
		{
			ioc->dbgio=io;
			_err e = ioc->IOC_State.Error;
			_dbg d = ioc->debug;
			ioc->debug = (_dbg)(d | DBG_ERR);
			ioc->IOC_State.Error=_NOERR;
			ioc->SetError(e);
			ioc->debug = d;
		}
			break;	
		default:
			return FR_INVALID_NAME;
	} 	
	return FR_OK;
}
//_________________________________________________________________________________
FRESULT _CLI::Decode(char *p) {
	char *sc[]={0,0,0,0,0,0,0,0};
	int i=0,n=0,len=1;
	switch(*p) {
		case '+':
			return DecodePlus(p);
		case '-':
			return DecodeMinus(p);
		case '?':
			return DecodeInq(p);
		case '=':
			return DecodeEq(p);
	}		
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
					_print("\r\n%-16s",p);
					if (fno.fattrib & AM_DIR)
						_print("%-8s","/");
					else
						_print("%-8d",(int)fno.fsize);	
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
			FIL	*f=new FIL;
			if(FRESULT err=f_open(f,sc[1],FA_READ))
				return err;	
			_print("\r\n");
			while(!f_eof(f)) 
				_print("%c",f_getc(f));
			f_close(f);
			delete f;
		}
	}
//__make directory_________________________________________________________________
	else if(!strncmp("mkdir",sc[0],len)) {
		if(n < 2)
			return FR_NO_FILE;
		if(FRESULT err=f_mkdir(sc[1]))
			return err;	
	}
//__color menu_____________________________________________________________________
	else if(!strncmp("color",sc[0],len)) {
		return _IOC::parent->ws2812.Decode(sc[1]);
	}
//__copy file______________________________________________________________________
	else if(!strncmp("copy",sc[0],len)) {
		char f[256];
		FIL	*f1=new FIL,
				*f2=new FIL;
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
		if(f_open(f1,sc[1],FA_READ)==FR_OK && f_open(f2,f,FA_CREATE_ALWAYS | FA_WRITE)==FR_OK) {
			while(!f_eof(f1))
				if(fputc(fgetc((FILE *)&f1),(FILE *)&f2)==EOF)
					break;
		}
		f_close(f1);
		f_close(f2);
		delete f1;
		delete f2;
		return FR_OK;
	}
//
////__entering new file____________________________________________________________
//						if(!strncmp("file",sc[0],len)) {
//							if(n == 2)
//								return(EnterFile(sc[1]));
//							else
//								return _PARSE_ERR_SYNTAX;
//						}
//__format flash drive_____________________________________________________________
//
	else if(!strncmp("format",sc[0],len)) {
		uint8_t	*workbuf;
		if(n < 2)
			return FR_NO_FILE;
		
		if(!strncmp("0:",sc[1],len)) {
			for(int i=FATFS_SECTOR; i<FATFS_SECTOR+FLASH_SECTOR_1*PAGE_COUNT;i+=FLASH_SECTOR_1) {
				FLASH_Erase(i,1);
				_print(".");
				_wait(100);
			}
		}		
		FRESULT err=f_mount(&fatfs,sc[1],1);
		if(err==FR_NO_FILESYSTEM)
			err=f_mkfs(sc[1],1,CLUSTER_SIZE,workbuf=new uint8_t[SECTOR_SIZE],SECTOR_SIZE*sizeof(int));
		else
			return err;
		delete workbuf;
		if(err!=FR_OK)
			return err;
	}
//__repack flash drive____________________________________________________________
	else if(!strncmp("pack",sc[0],len)) {
		ff_pack(EOF);
	}
//__dump memory contents___________________________________________________________
	else if(!strncmp("dump",sc[0],len)) {
		dumpHex(strtoul( sc[1],NULL,0),strtoul( sc[2],NULL,0));
	}
	else if(!strncmp("wait",sc[0],len)) {
		_wait(atoi(sc[1]));
	}
//__file line edit/add ___________________________________________________________
	else if(!strncmp("file",sc[0],len)) {
		class _ENTERFILE : public _TERM, public _FAT {
			private:
				FIL *f;
			public:
				FRESULT err;
				_ENTERFILE(char *filename) {
					err=f_open(f=new FIL, filename, FA_OPEN_ALWAYS | FA_WRITE);
					if(err==FR_OK) {
						f_lseek(f,f_size(f));
						Newline();
					}
				};
				~_ENTERFILE(void) {
					f_close(f);
					delete f;
				};
				virtual FRESULT Decode(char *c) {
					f_printf(f,"%s\r\n",c);
					return FR_OK;
				};
				virtual void Newline(void) {
					_print("\r\n");
				};	
		} efile(sc[1]);
		
		while(efile.Parse() && efile.err == FR_OK) {
			_wait(2);
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
	} else if(!strncmp("@",sc[0],1)) {
		Batch(++sc[0]);
	} else if(!strncmp("=d",sc[0],2)) {
		float k=0.0006250;
		float fo=15e6/(12+56)/2;
		float t=atof(sc[1])/(1-expf(-fo*k*(float)atof(sc[1])));
		float tt=t-atof(sc[1]);
		float ut=0;
		
		for(int n=0; n<10; ++n) {	
			float tt=t-atof(sc[1]);
			ut+=1-fo*k*t*expf(-fo*k*t)-expf(-fo*k*t);
			ut-=1-fo*k*tt*expf(-fo*k*tt)-expf(-fo*k*tt);
			t+= atof(sc[2]);
		// double t = Ton / (1 - exp(-fo * k * Ton));
		// 9.06666666666666666666666 us
			_print("\r\n t=%f, u(t)=%f",t,ut);	
		}			
		return FR_OK;
		
	} /*else if(!strncmp("+D",sc[0],2)) {
		for(char *c=sc[1]; c && *c;)
			ioc->debug = (_dbg(ioc->debug | (1<<strtoul(c,&c,10))));
	} else if(!strncmp("-D",sc[0],2)) {
		for(char *c=sc[1]; c && *c;)
			ioc->debug = (_dbg(ioc->debug | ~(1<<strtoul(c,&c,10))));
	} else if(!strncmp("+E",sc[0],2)) {
	} else if(!strncmp("-E",sc[0],2)) {
	} else if(!strncmp("?P",sc[0],2)) {
		_proc_list();
	} */else {
		if(n) {
			for(i=0; i<n; ++i)
				_print(" %s",sc[i]);
			return FR_INVALID_NAME;
		}
	}
	return FR_OK;
}
/*******************************************************************************
* Function Name	:
* Description		:
* Output				:
* Return				:
*******************************************************************************/
void _CLI::Batch(char *filename) {
			FIL *f=new FIL;
			if(f_open(f,filename,FA_READ) == FR_OK) {
				while(!f_eof(f))
					Parse(f);
				f_close(f);	
			}
			delete f;
}
//_________________________________________________________________________________
int	_CLI::wcard(char *t, char *s) {
			return *t-'*' ? *s ? (*t=='?') | (toupper(*s)==toupper(*t)) && wcard(t+1,s+1) : !*t : wcard(t+1,s) || (*s && wcard(t,s+1));
}
//_________________________________________________________________________________
string days[]={"Mon","Tue","Wed","Thu","Fri","Sat","Sun"};
string months[]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
void	_CLI::printRtc() {
RTC_TimeTypeDef t;
RTC_DateTypeDef d;
		HAL_RTC_GetTime(&hrtc,&t,RTC_FORMAT_BIN);
		HAL_RTC_GetDate(&hrtc,&d,RTC_FORMAT_BIN);
		_print("%4s,%3d-%3s-%d,%3d:%02d:%02d",days[d.WeekDay-1].c_str(),d.Date,months[d.Month-1].c_str(),d.Year,t.Hours,t.Minutes,t.Seconds);
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
							_print("\r\n%s",lfn);
							if (fno.fattrib & AM_DIR)
								_print("/");
							else
								_print("%*d",32-strlen(lfn),(int)fno.fsize);
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
