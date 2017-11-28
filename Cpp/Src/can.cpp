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
	filter_count=0;
	io=NULL;

	canFilterCfg(idIOC_State,	0x780);
	canFilterCfg(idEC20_req,	0x780);
	canFilterCfg(idEM_ack,		0x7ff);
	canFilterCfg(idBOOT,			0x7ff);
	
	_proc_add((void *)task,this,(char *)"can task",0);
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
int _CAN::SendRemote() {
	CanTxMsgTypeDef	tx={idCAN2COM,0,CAN_ID_STD,CAN_RTR_DATA,0,0,0,0,0,0,0,0,0};	
	int i;	
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
void _CAN::Send(CanTxMsgTypeDef *msg) {
	_buffer_push(canBuffer->tx,msg,sizeof(CanTxMsgTypeDef));
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
			while(SendRemote() != __CtrlE) 
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
void	_CAN::Poll() {
	_IOC *ioc=_IOC::parent;
	CanRxMsgTypeDef*	rx=hcan->pRxMsg;
	CanTxMsgTypeDef*	tx=hcan->pTxMsg;
	_io* temp=_stdio(io);										//remote console printout !!!

	while(1) {
		if(((hcan->Instance->TSR&CAN_TSR_TME0) != CAN_TSR_TME0) && \
			 ((hcan->Instance->TSR&CAN_TSR_TME1) != CAN_TSR_TME1) && \
			 ((hcan->Instance->TSR&CAN_TSR_TME2) != CAN_TSR_TME2)) 	
					break;
		if(_buffer_pull(canBuffer->tx,tx,sizeof(CanTxMsgTypeDef)))
			HAL_CAN_Transmit_IT(hcan);
		else if(remote) {
			tx->DLC=_buffer_pull(remote->io->tx,tx->Data,8);
			tx->StdId=idCOM2CAN;
			if(tx->DLC)
				HAL_CAN_Transmit_IT(hcan);
			else
				break;
		} else
			break;
	}

	if(_buffer_pull(canBuffer->rx,rx,sizeof(CanRxMsgTypeDef))) {
		switch(rx->StdId) {
			case idIOC_State:
				if(rx->DLC) {
					switch((_State)rx->Data[0]) {
						case	_STANDBY:
							ioc->IOC_State.State = _STANDBY;
							ioc->IOC_State.Error = _NOERR;
//							ioc->Submit("@standby.led");
							_SYS_SHG_ENABLE;
							break;
						case	_READY:
							if(ioc->IOC_State.State == _STANDBY || ioc->IOC_State.State == _ACTIVE) {
								ioc->IOC_State.State = _READY;
//								ioc->Submit("@ready.led");
							} else
								ioc->SetError(_illstatereq);
							break;
						case	_ACTIVE:
							if(ioc->IOC_State.State == _READY) {
								ioc->IOC_State.State = _ACTIVE;
//								ioc->Submit("@active.led");
							} else
								ioc->SetError(_illstatereq);
							break;
						case	_ERROR:
							ioc->IOC_State.State = _ERROR;
//							ioc->Submit("@error.led");
							_SYS_SHG_DISABLE;
							break;
						default:
							ioc->SetError(_illstatereq);
							break;
					}
				}
				ioc->IOC_State.Send();
				break;
			case idIOC_SprayParm:
				ioc->spray.AirLevel		= std::min(10,(int)rx->Data[0]);
				ioc->spray.WaterLevel	= std::min(10,(int)rx->Data[1]);
				if(rx->Data[2])
					ioc->spray.mode.On=true;
				else {
					if(ioc->spray.mode.On==false)
						ioc->spray.timeout=HAL_GetTick() + _SPRAY_READY_T;
					ioc->spray.mode.On=false;
				}
			break;
//______________________________________________________________________________________							
			case idCAN2FOOT:
extern _io*		__com3;
							{
_io*						io=_stdio(__com3);
								for(int i=0; i<rxm.DLC; ++i)
									while(putchar(rxm.Data[i]) == EOF)
										_wait(10,_thread_loop);
								_stdio(io);
							}								
							break;
//______________________________________________________________________________________							
							case idEC20_req:
								timeout=__time__+_EC20_EM_DELAY;			
							break;
//______________________________________________________________________________________							
							case idEM_ack:
								timeout=0;
								lm->IOC_FootAck.Send();	
							break;
//______________________________________________________________________________________							
							case idBOOT:
								if(rxm.Data[0]==0xAA)
									while(1);
							break;

				
				
				
				
				
				
				
				
				
				
				
				
				
				
				
				
				
			case idCAN2COM:
				if(remote)
					_buffer_push(remote->io->rx,rx->Data,rx->DLC);
			break;
			case idCOM2CAN:
				for(int i=0; i < rx->DLC;++i)
					_print("%c",rx->Data[i]);
			break;
			default:
				Newline();
				_print(" < %02X ",rx->StdId);
				for(int i=0; i < rx->DLC;++i)
					_print(" %02X",rx->Data[i]);
				Newline();	
			break;
		}
	}
	_stdio(temp);
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
