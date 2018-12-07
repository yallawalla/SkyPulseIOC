#include <stdlib.h>
#include "ioc.h"
CAN_HandleTypeDef *_CAN::hcan;
_io 							*_CAN::io,*_CAN::ioFsw;
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
_CAN::_CAN(CAN_HandleTypeDef *handle) {
	remote = new _CLI();
	canBuffer	=	_io_init(100*(sizeof(CAN_RxHeaderTypeDef)+8), 100*(sizeof(CAN_TxHeaderTypeDef)+8));
	hcan = handle;
	error = _NOERR;
	
	filter_count=ecTimeout=dlTimeout=0;
	
	canFilterCfg(idIOC_State,	0x7C0, idBOOT,			0x7ff);
	canFilterCfg(idEM_ack,		0x7ff, idEC20_req,	0x7ff);
	canFilterCfg(idDL_Timing,	0x7f0, idDL_Timing,	0x7f0);
	HAL_CAN_ActivateNotification(hcan,CAN_IT_RX_FIFO0_MSG_PENDING);
	HAL_CAN_ActivateNotification(hcan,CAN_IT_TX_MAILBOX_EMPTY);
	HAL_CAN_Start(hcan);
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void	_CAN::Send(int id,  void *data, int len) {
	CAN_TxHeaderTypeDef	hdr={0,0,CAN_ID_STD,CAN_RTR_DATA,0,DISABLE};
	uint32_t mailbox;
	hdr.StdId=id;
	hdr.DLC=len;

	if(HAL_CAN_AddTxMessage(hcan, &hdr, (uint8_t *)data, &mailbox) != HAL_OK) {
		_buffer_push(canBuffer->tx,&hdr,sizeof(CAN_TxHeaderTypeDef));
		_buffer_push(canBuffer->tx,data,len*sizeof(uint8_t));
	}

	Debug(DBG_CAN_TX,"\r\n%04d: > %02X ",__time__ % 1000,id);
//	Debug(DBG_CAN_TX,"\r\n> %02X ",id);
	uint8_t *p=(uint8_t *)data;
	for(int i=0; i < len; ++i)
		Debug(DBG_CAN_TX," %02X",*p++);
	Debug(DBG_CAN_TX,"\r\n");
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
int 	_CAN::SendRemote(int stdid) {
	uint8_t data[8];
	int			m,n=0;
	while(n<8) {
		m=getchar();
		if(m==EOF || m==__CtrlE)
			break;
		else
			data[n++]=m;
	}
	if(n)
		Send(stdid,data,n);
	return m;
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
_err 	_CAN::Status() {
	_err e=error;
	error=_NOERR;
	return e;
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void 	_CAN::Status(_err e) {
	error = error | e;
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void	_CAN::Newline(void) {
	io=_stdio(NULL);
	_stdio(io);
	_print("\r\ncan>");
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
int		_CAN::Fkey(int t) {
	switch(t) {
		
		case __CtrlE:
			_print("remote console...\r\n");
			while(SendRemote(idCAN2COM) != __CtrlE) 
				_wait(2);
			_print("close...");
			Newline();
			break;

		case __f8:
		case __F8:
			io=_stdio(NULL);
			return __F12;		
		
		default:
			return t;
	}
	return EOF;
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void	_CAN::pollRx(void *v) {
	CAN_RxHeaderTypeDef		rx;
	uint8_t								data[8];
	_IOC*									ioc = static_cast<_IOC *>(v);
	if(_buffer_pull(canBuffer->rx,&rx,sizeof(CAN_RxHeaderTypeDef))) {
		_buffer_pull(canBuffer->rx,data,rx.DLC*sizeof(uint8_t));
//______________________________________________________________________________________				
		Debug(DBG_CAN_RX,"\r\n%04d: < %02X ",__time__ % 1000,rx.StdId);
//		Debug(DBG_CAN_RX,"\r\n< %02X ",rx.StdId);
		for(int i=0; i < rx.DLC; ++i)
			Debug(DBG_CAN_RX," %02X",data[i]);
		Debug(DBG_CAN_RX,"\r\n");
//______________________________________________________________________________________
		switch(rx.StdId) {
			case idIOC_State:
				if(rx.DLC) {
					ioc->SetState(data);
					if(ioc->IOC_State.State == _ACTIVE)
						ecTimeout=dlTimeout=__time__ + _EC20_MAX_PERIOD;
					else {
						ecTimeout=dlTimeout=0;
					}
				}
				ioc->IOC_State.Send();
				break;
//______________________________________________________________________________________
			case idIOC_SprayParm:
				ioc->spray.AirLevel		= std::min(10,(int)data[0]);
				ioc->spray.WaterLevel	= std::min(10,(int)data[1]);
				if(data[2]==0) {
					if(ioc->spray.mode.Air==false && ioc->spray.mode.Water==false)
						ioc->spray.readyTimeout=__time__ + _SPRAY_READY_T;
				}
				ioc->spray.mode.Air=data[2] & 1;
				ioc->spray.mode.Water=data[2] & 2;
			break;
//______________________________________________________________________________________
			case idIOC_AuxReq:
				ioc->IOC_Aux.Flow = ioc->pump.Flow/(2200/300);
				ioc->IOC_Aux.Pump = ioc->pump.Rpm(100);
				ioc->IOC_Aux.Temp = ioc->pump.Th2o();
				ioc->IOC_Aux.Fan = ioc->fan.Rpm(100);
				ioc->IOC_Aux.Send();
			break;
//______________________________________________________________________________________
			case idIOC_VersionReq:
				ioc->IOC_VersionAck.Send();
			break;
//______________________________________________________________________________________
			case idEM_ack:
				if(ioc->IOC_State.State == _ACTIVE) {
					ecTimeout=__time__ + _EC20_MAX_PERIOD;
				} else {
					ecTimeout=0;
					Status(_illENMack);			
				}
				ioc->IOC_FootAck.Send();
			break;
//______________________________________________________________________________________
			case idEC20_req:
				if(ioc->IOC_State.State == _ACTIVE) {
					ecTimeout=__time__ + _EC20_ENM_DELAY;
					dlTimeout=0;
				}
				else {
					ecTimeout=0;
					Status(_illEC20req);	
				}
			break;
//______________________________________________________________________________________
			case idDL_Limits: {
				DL_Limits *p=(DL_Limits *)data;
				ioc->diode.setLimits(p->L0,p->L1,p->mode);
				
				if(ioc->IOC_State.State == _ACTIVE) {
					dlTimeout=__time__ + _DL_POLL_DELAY;
					ecTimeout=0;
				}
				ioc->IOC_FootAck.Send();
			}
			break;
//______________________________________________________________________________________
			case idDL_State: {
				DL_State *p=(DL_State *)data;
				if(p->State == _CALIBRATE) {
					ioc->error_mask = ioc->error_mask | _DLpowerCh1 | _DLpowerCh2 | _ENMtimeout | _DLtimeout;
				}
			}
			break;				
//______________________________________________________________________________________
			case idDL_Timing: {
				DL_Timing *p=(DL_Timing *)data;
				ioc->diode.setTiming(p->Ton,p->Toff);
			}
			break;				
//______________________________________________________________________________________
			case idIOC_Footreq:
				ioc->IOC_FootAck.Send();
			break;
//______________________________________________________________________________________
			case idIOC_FootAck:
			break;
//______________________________________________________________________________________
			case idBOOT:
				if(data[0]==0xAA)
					while(1);
				break;
//______________________________________________________________________________________							
			case idFOOT2CAN:
				Send(idCAN2FOOT,data,rx.DLC);
			break;
//______________________________________________________________________________________							
			case idCAN2FOOT:
				if(ioFsw)
					while(rx.DLC && !_buffer_push(ioFsw->tx,data,rx.DLC))
						_wait(2);
			break;
//______________________________________________________________________________________
			case idCAN2COM:
				while(rx.DLC && !_buffer_push(remote->io->rx,data,rx.DLC))
					_wait(2);
			break;
//______________________________________________________________________________________
			case idCOM2CAN:
				while(rx.DLC && !_buffer_push(io->tx,data,rx.DLC))
					_wait(2);
			break;
//______________________________________________________________________________________
			default:
			break;
		}
	}
//______________________________________________________________________________________
//
//	DL & EC20 timeout check
//______________________________________________________________________________________
	if(ecTimeout && __time__ > ecTimeout) {
		ecTimeout=0;
		Status(_ENMtimeout);	
	}
//______________________________________________________________________________________
//
//	DL timeout check
//______________________________________________________________________________________
	if(dlTimeout && __time__ > dlTimeout) {
		dlTimeout=0;
		Status(_DLtimeout);	
	}
//__CAN console processing______________________________________________________________
	if(remote) {
		uint8_t	data[8],n;
		n=_buffer_pull(remote->io->tx,data,8);
		if(n)
			Send(idCOM2CAN,data,n);
	}
}
/*******************************************************************************
* Function Name  : CAN_Initialize
* Description    : Configures the CAN, transmit and receive using interrupt.
* Input          : None
* Output         : None
* Return         : PASSED if the reception is well done, FAILED in other case
*******************************************************************************/
void	_CAN::canFilterCfg(int id1, int mask1, int id2, int mask2) {
CAN_FilterTypeDef  sFilterConfig;

  sFilterConfig.FilterBank = 14 + filter_count++;
  sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
  sFilterConfig.FilterScale = CAN_FILTERSCALE_16BIT;
	sFilterConfig.FilterIdHigh = id1<<5;
	sFilterConfig.FilterMaskIdHigh = mask1<<5;	
	sFilterConfig.FilterIdLow =  id2<<5;
	sFilterConfig.FilterMaskIdLow = mask2<<5;
  sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
  sFilterConfig.FilterActivation = ENABLE;
  sFilterConfig.SlaveStartFilterBank = 0;
	HAL_CAN_ConfigFilter(hcan, &sFilterConfig) ;
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
FRESULT	_CAN::Decode(char *c) {
	CAN_RxHeaderTypeDef	rxHdr={0,0,CAN_ID_STD,CAN_RTR_DATA,0,0,0};
	uint8_t							data[8];
	uint32_t						stdid,n;
	switch(*c)  {
		case '<': 
			rxHdr.StdId=strtoul(++c,&c,16);
			do {
				while(*c == ' ') ++c;
				for(rxHdr.DLC=0; *c && rxHdr.DLC < 8; ++rxHdr.DLC)
					data[rxHdr.DLC]=strtoul(c,&c,16);
				_buffer_push(canBuffer->rx,&rxHdr,sizeof(CAN_RxHeaderTypeDef));
				_buffer_push(canBuffer->rx,data,rxHdr.DLC*sizeof(uint8_t));
			} while(*c);
			break;

		case '>': 
			stdid=strtoul(++c,&c,16);
			do {
				while(*c == ' ') ++c;
				for(n=0; *c && n < 8; ++n)
					data[n]=strtoul(c,&c,16);
				Send(stdid,data,n);
			} while(*c);
			break;

		case 'l': 
			HAL_CAN_DeInit(hcan);
			hcan->Init.Mode = CAN_MODE_LOOPBACK;
			HAL_CAN_Init(hcan);
				
			filter_count=ecTimeout=dlTimeout=0;
			canFilterCfg(idIOC_State,	0x780, idBOOT,			0x7ff);
			canFilterCfg(idEM_ack,		0x7ff, idEC20_req,	0x7ff);
			HAL_CAN_ActivateNotification(hcan,CAN_IT_RX_FIFO0_MSG_PENDING);
			HAL_CAN_ActivateNotification(hcan,CAN_IT_TX_MAILBOX_EMPTY);
			HAL_CAN_Start(hcan);
			break;

		case 'f': 
			for(c=strchr(c,' '); c && *c;) {
				int i=strtoul(++c,&c,16);
				int j=strtoul(++c,&c,16);
				int k=strtoul(++c,&c,16);
				int l=strtoul(++c,&c,16);
				canFilterCfg(i,j,k,l);
			}
			for(int i=0; i<28; ++i)
				if(CAN1->FA1R & (1<<i)) {
					_print("\r\n    %04X,%04X",(CAN1->sFilterRegister[i].FR1 & 0xffff)>>5,(CAN1->sFilterRegister[i].FR1) >> 21);
					_print("\r\n%2d%c %04X,%04X",i,')',(CAN1->sFilterRegister[i].FR2 & 0xffff)>>5,(CAN1->sFilterRegister[i].FR2) >> 21);
				}

			break;
				
		case 'w': 
			_wait(atoi(++c));
		break;		
		
		case '@': 
			Batch(++c);
		break;
	
		default:
			if(*c)
				return FR_INVALID_PARAMETER;
	}
	return FR_OK;
}


