#include	"ioc.h"
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
_CAN::_CAN(CAN_HandleTypeDef *handle) {
	remote = new _CLI(NULL);
	hcan = handle;
	hcan->pRxMsg=new CanRxMsgTypeDef;
	hcan->pTxMsg=new CanTxMsgTypeDef;
	canBuffer	=	_io_init(100*sizeof(CanRxMsgTypeDef), 100*sizeof(CanTxMsgTypeDef));
	HAL_CAN_Receive_IT(hcan,CAN_FIFO0);
	filter_count=timeout=0;
	io=ioFsw=NULL;

	canFilterCfg(idIOC_State,	0x780);
	canFilterCfg(idEC20_req,	0x780);
	canFilterCfg(idEM_ack,		0x7ff);
	canFilterCfg(idBOOT,			0x7ff);

	_proc_add((void *)taskRx,this,(char *)"can rx",0);
	_proc_add((void *)taskTx,this,(char *)"can tx",1);
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
int _CAN::SendRemote(int stdid) {
	CanTxMsgTypeDef	tx={0,0,CAN_ID_STD,CAN_RTR_DATA,0,0,0,0,0,0,0,0,0};	
	int i;	
	tx.StdId=stdid;
	while(tx.DLC  < 8) {
		i=getchar();
		if(i==EOF || i==__CtrlE)
			break; 
		tx.Data[tx.DLC++] = i;
	}
	if(tx.DLC > 0)
		Send(&tx);
	return i;
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void _CAN::Newline(void) {
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
int	_CAN::Fkey(int t) {
	switch(t) {
		case __CtrlE:
			_print("remote desktop...\r\n");
			while(SendRemote(idCOM2CAN) != __CtrlE) 
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
FRESULT _CAN::Decode(char *c) {
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
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void	_CAN::pollTx() {
//	CanTxMsgTypeDef*	tx=hcan->pTxMsg;
////______________________________________________________________________________________					
//	while(1) {
//		if(((hcan->Instance->TSR&CAN_TSR_TME0) != CAN_TSR_TME0) && \
//			 ((hcan->Instance->TSR&CAN_TSR_TME1) != CAN_TSR_TME1) && \
//			 ((hcan->Instance->TSR&CAN_TSR_TME2) != CAN_TSR_TME2)) 	
//					return;
//		if(_buffer_pull(canBuffer->tx,tx,sizeof(CanTxMsgTypeDef)))
//			HAL_CAN_Transmit_IT(hcan);
//		else if(remote) {
//			tx->DLC=_buffer_pull(remote->io->tx,tx->Data,8);
//			tx->StdId=idCAN2COM;
//			if(tx->DLC)
//				HAL_CAN_Transmit_IT(hcan);
//			else
//				return;
//		} else
//			return;
//	}
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void _CAN::Send(CanTxMsgTypeDef *msg) {
	
	_buffer_push(canBuffer->tx,msg,sizeof(CanTxMsgTypeDef));
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void	_CAN::pollRx() {
	CanRxMsgTypeDef*	rx=hcan->pRxMsg;
	_IOC*							ioc=_IOC::parent;
	
	if(_buffer_pull(canBuffer->rx,rx,sizeof(CanRxMsgTypeDef))) {
		_io*	temp=_stdio(io);										//remote console printout !!!
		switch(rx->StdId) {
			case idIOC_State:
				if(rx->DLC) {
					switch((_State)rx->Data[0]) {
						case	_STANDBY:
							if(ioc->IOC_State.State == _ERROR)
								ioc->IOC_State.Error = _NOERR;
							ioc->IOC_State.State = _STANDBY;
							ioc->pump.Enable();
//							ioc->Submit("@standby.led");
							_SYS_SHG_ENABLE;
							break;
						case	_READY:
							ioc->IOC_State.State = _READY;
							ioc->pump.Enable();
//							ioc->Submit("@ready.led");
							break;
						case	_ACTIVE:
							ioc->IOC_State.State = _ACTIVE;
							ioc->pump.Enable();
//							ioc->Submit("@active.led");
							break;
						case	_ERROR:
							ioc->IOC_State.State = _ERROR;
//							ioc->Submit("@error.led");
							_SYS_SHG_DISABLE;
							break;
						default:
							break;
					}
				}
				ioc->IOC_State.Send();
				break;
			case idIOC_SprayParm:
				ioc->spray.AirLevel		= std::min(10,(int)rx->Data[0]);
				ioc->spray.WaterLevel	= std::min(10,(int)rx->Data[1]);
				if(rx->Data[2]==0) {
					if(ioc->spray.mode.Air==false && ioc->spray.mode.Water==false)
						ioc->spray.readyTimeout=__time__ + _SPRAY_READY_T;
				}
				ioc->spray.mode.Air=rx->Data[2] & 1;
				ioc->spray.mode.Water=rx->Data[2] & 2;
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
				if(rx->Data[0]==0xAA)
					while(1);
				break;
//______________________________________________________________________________________							
			case idFOOT2CAN:
				rx->StdId=idCAN2FOOT;
				Send((CanTxMsgTypeDef *)rx);
			break;
//______________________________________________________________________________________							
			case idCAN2FOOT: {
				_io*	io=_stdio(ioFsw);
				for(int i=0; i<rx->DLC; ++i)
					putchar(rx->Data[i]);
				_stdio(io);
			} break;
//______________________________________________________________________________________
			case idCOM2CAN:
				if(remote)
					for(int i=0; i<rx->DLC; ++i)
						while(!_buffer_push(remote->io->rx,&rx->Data[i],1))
							_wait(2);
			break;
//______________________________________________________________________________________
			case idCAN2COM:
				for(int i=0; i < rx->DLC;++i)
					putchar(rx->Data[i]);
			break;
//______________________________________________________________________________________
			default:
				Newline();
				_print(" < %02X ",rx->StdId);
				for(int i=0; i < rx->DLC;++i)
					_print(" %02X",rx->Data[i]);
				Newline();	
			break;
		}
	_stdio(temp);
	}
//______________________________________________________________________________________					
	if(timeout && __time__ > timeout) {
		timeout=0;
		ioc->SetError(_energyMissing);	
	}
}
/*******************************************************************************
* Function Name  : CAN_Initialize
* Description    : Configures the CAN, transmit and receive using interrupt.
* Input          : None
* Output         : None
* Return         : PASSED if the reception is well done, FAILED in other case
*******************************************************************************/
void _CAN::canFilterCfg(int id, int mask) {
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

