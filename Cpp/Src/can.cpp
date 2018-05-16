#include	"ioc.h"
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
	hcan = handle;
	hcan->pRxMsg=new CanRxMsgTypeDef;
	canBuffer	=	_io_init(100*sizeof(CanRxMsgTypeDef), 100*sizeof(CanTxMsgTypeDef));
	HAL_CAN_Receive_IT(hcan,CAN_FIFO0);
	filter_count=timeout=0;

	canFilterCfg(idIOC_State,	0x780);
	canFilterCfg(idEC20_req,	0x780);
	canFilterCfg(idEM_ack,		0x7ff);
	canFilterCfg(idBOOT,			0x7ff);
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void	_CAN::Send(CanTxMsgTypeDef *msg) {
	Send(msg->StdId,msg->Data,msg->DLC);
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void	_CAN::Send(int id, void *data, int len) {
	CanTxMsgTypeDef	msg={0,0,CAN_ID_STD,CAN_RTR_DATA,sizeof(_IOC_State),0,0,0,0,0,0,0,0};
	msg.StdId=id;
	msg.DLC=len;
	memcpy(msg.Data,data,len);
	hcan->pTxMsg=&msg;
	if(HAL_CAN_Transmit_IT(hcan) != HAL_OK)
		_buffer_push(canBuffer->tx,&msg,sizeof(CanTxMsgTypeDef));

	Debug(DBG_CAN_TX,"\r\n> %02X ",id);
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
	CanRxMsgTypeDef		rx;
	_IOC*							ioc=static_cast<_IOC *>(v);
	while(_buffer_pull(canBuffer->rx,&rx,sizeof(CanRxMsgTypeDef))) {
//______________________________________________________________________________________							
		Debug(DBG_CAN_RX,"\r\n< %02X ",rx.StdId);
		for(int i=0; i < rx.DLC; ++i)
			Debug(DBG_CAN_RX," %02X",rx.Data[i]);
		Debug(DBG_CAN_RX,"\r\n");
//______________________________________________________________________________________							
		switch(rx.StdId) {
			case idIOC_State:
				if(rx.DLC)
					ioc->SetState((_State)rx.Data[0]);
				ioc->IOC_State.Send();
				break;
//______________________________________________________________________________________							
			case idIOC_SprayParm:
				ioc->spray.AirLevel		= std::min(10,(int)rx.Data[0]);
				ioc->spray.WaterLevel	= std::min(10,(int)rx.Data[1]);
				if(rx.Data[2]==0) {
					if(ioc->spray.mode.Air==false && ioc->spray.mode.Water==false)
						ioc->spray.readyTimeout=__time__ + _SPRAY_READY_T;
				}
				ioc->spray.mode.Air=rx.Data[2] & 1;
				ioc->spray.mode.Water=rx.Data[2] & 2;
			break;
//______________________________________________________________________________________							
			case idIOC_AuxReq:
				ioc->IOC_Aux.Temp = ioc->Th2o();
				ioc->IOC_Aux.Send();
			break;
//______________________________________________________________________________________
			case idEC20_req:
				timeout=__time__+_EC20_EM_DELAY;
			break;
//______________________________________________________________________________________
			case idEM_ack:
				timeout=0;
			case idIOC_Footreq:
				ioc->IOC_FootAck.Send();
			break;
//______________________________________________________________________________________
			case idBOOT:
				if(rx.Data[0]==0xAA)
					while(1);
				break;
//______________________________________________________________________________________							
			case idFOOT2CAN:
				Send(idCAN2FOOT,rx.Data,rx.DLC);
			break;
//______________________________________________________________________________________							
			case idCAN2FOOT:
				while(rx.DLC && !_buffer_push(ioFsw->tx,rx.Data,rx.DLC))
					_wait(2);
			break;
//______________________________________________________________________________________
			case idCAN2COM:
				while(rx.DLC && !_buffer_push(remote->io->rx,rx.Data,rx.DLC))
					_wait(2);
			break;
//______________________________________________________________________________________
			case idCOM2CAN:
				while(rx.DLC && !_buffer_push(io->tx,rx.Data,rx.DLC))
					_wait(2);
			break;
//______________________________________________________________________________________
			default:
			break;
		}
	}
//______________________________________________________________________________________					
	if(timeout && __time__ > timeout) {
		timeout=0;
		ioc->SetError(_energyMissing);	
	}
//______________________________________________________________________________________
	if(remote) {
		CanTxMsgTypeDef	tx={idCOM2CAN,0,CAN_ID_STD,CAN_RTR_DATA,0,0,0,0,0,0,0,0,0};
		tx.DLC=_buffer_pull(remote->io->tx,tx.Data,8);
		if(tx.DLC)
			Send(&tx);
	}
}
/*******************************************************************************
* Function Name  : CAN_Initialize
* Description    : Configures the CAN, transmit and receive using interrupt.
* Input          : None
* Output         : None
* Return         : PASSED if the reception is well done, FAILED in other case
*******************************************************************************/
void	_CAN::canFilterCfg(int id, int mask) {
CAN_FilterConfTypeDef  sFilterConfig;

  sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
  sFilterConfig.FilterScale = CAN_FILTERSCALE_16BIT;
  sFilterConfig.FilterFIFOAssignment = 0;
  sFilterConfig.BankNumber = 14;
  sFilterConfig.FilterActivation = ENABLE;
	if(filter_count % 2) {
		sFilterConfig.FilterIdHigh = id<<5;
		sFilterConfig.FilterMaskIdHigh = mask<<5;	
	} else {
		sFilterConfig.FilterIdLow =  id<<5;
		sFilterConfig.FilterMaskIdLow = mask<<5;
	}
	sFilterConfig.FilterNumber = sFilterConfig.BankNumber + filter_count++;
	HAL_CAN_ConfigFilter(hcan, &sFilterConfig);
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
FRESULT	_CAN::Decode(char *c) {
	CanRxMsgTypeDef	rx={0,0,CAN_ID_STD,CAN_RTR_DATA,0,0,0,0,0,0,0,0,0};
	CanTxMsgTypeDef	tx={0,0,CAN_ID_STD,CAN_RTR_DATA,0,0,0,0,0,0,0,0,0};		

	switch(*c)  {
		case '<': 
			rx.StdId=strtol(++c,&c,16);
			do {
				while(*c == ' ') ++c;
				for(rx.DLC=0; *c && rx.DLC < 8; ++rx.DLC)
					rx.Data[rx.DLC]=strtol(c,&c,16);
				_buffer_push(canBuffer->rx,&rx,sizeof(CanRxMsgTypeDef));
			} while(*c);
			break;

		case '>': 
			tx.StdId=strtol(++c,&c,16);
			do {
				while(*c == ' ') ++c;
				for(tx.DLC=0; *c && tx.DLC < 8; ++tx.DLC)
					tx.Data[tx.DLC]=strtol(c,&c,16);
				Send(&tx);
			} while(*c);
			break;

		case 'l': 
			hcan->Init.Mode = CAN_MODE_LOOPBACK;
			HAL_CAN_Init(hcan);
			break;

		case 'f': 
			for(c=strchr(c,' '); c && *c;) {
				int i=strtoul(++c,&c,16);
				int j=strtoul(++c,&c,16);
				canFilterCfg(i,j);
			}
			for(int i=0; i<28; ++i)
				if(CAN1->FA1R & (1<<i)) {
					if(i % 2)
						_print("\r\n%d%c %04X,%04X",i,')',(CAN1->sFilterRegister[i].FR2 & 0xffff)>>5,(CAN1->sFilterRegister[i].FR2) >> 21);
					else
						_print("\r\n%d%c %04X,%04X",i,')',(CAN1->sFilterRegister[i].FR1 & 0xffff)>>5,(CAN1->sFilterRegister[i].FR1) >> 21);
				}

			break;
	
		default:
			if(*c)
				return FR_INVALID_PARAMETER;
	}
	return FR_OK;
}


