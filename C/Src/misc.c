#include	"stm32f4xx_hal.h"
#include 	"io.h"
#include 	"proc.h"
#include 	"misc.h"
#include 	"ff.h"
#include 	"diskio.h"
#include 	"usbd_cdc_if.h"
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
HAL_StatusTypeDef	FLASH_Program(uint32_t Address, uint32_t Data) {
	HAL_StatusTypeDef status;
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPERR  | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR );
	if(*(uint32_t *)Address !=  Data) {
		HAL_FLASH_Unlock();
		do
			status=HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,Address,Data);
		while(status == HAL_BUSY);
		HAL_FLASH_Lock();
	}	
	return status;
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
HAL_StatusTypeDef	FLASH_Erase(uint32_t sector, uint32_t n) {
FLASH_EraseInitTypeDef EraseInitStruct;
HAL_StatusTypeDef ret;
uint32_t	SectorError;
	HAL_FLASH_Unlock();
  EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
  EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
  EraseInitStruct.Sector = sector;
  EraseInitStruct.NbSectors = n;
  ret=HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError);
  HAL_FLASH_Lock(); 
	return ret;
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
#define _MAXBYTESLINE 16
void	dumpHex(int a, int n) {
	unsigned int k;
	__print("\r\n:02000004%04X%02X\r\n",(a>>16),-(2+4+((a>>16)/256)+((a>>16) % 256)) & 255);
	while(n) {
		int	sum = _MAXBYTESLINE+(a & 0xffff)/256+(a & 0xff);
		__print(":%02X%04X00",_MAXBYTESLINE,(a & 0xffff));
		for(k = 0; k<_MAXBYTESLINE; ++k) {
			__print("%02X",(*(unsigned char *)a & 0xff));
			sum += *(unsigned char *)a;
			if(((++a) & 0xffff) == 0 || --n == 0)
				break;
		}
		__print("%02X\r\n",-sum & 0xff);
		if(n && (a & 0xffff) == 0) {
			__print(":02000004%04X,%02X\r\n",(a>>16),-(2+4+((a>>16)/256)+((a>>16) % 256)) & 255);
		}
	}
	__print(":00000001FF\r\n");
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void	poll_uart(_io *io) {
UART_HandleTypeDef *huart=io->huart;
	io->rx->_push = (char *)&huart->pRxBuffPtr[huart->RxXferSize - huart->hdmarx->Instance->NDTR];	
	if(huart->gState == HAL_UART_STATE_READY) {
		int len;
		if(!huart->pTxBuffPtr)
			huart->pTxBuffPtr=malloc(io->tx->size);
		do {
			len=_buffer_pull(io->tx, huart->pTxBuffPtr, io->tx->size);
			if(len)
				HAL_UART_Transmit_DMA(huart, huart->pTxBuffPtr, len);
		} while(len > 0);
	}
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
_io* init_uart(UART_HandleTypeDef *huart, int sizeRx, int sizeTx) {
	_io* io=_io_init(sizeRx,sizeTx);
	if(io && huart) {
		io->huart=huart;
		HAL_UART_Receive_DMA(huart,(uint8_t*)io->rx->_buf,io->tx->size);
		_proc_add(poll_uart,io,"uart",0);
	}
	return io;
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
	if(htim->Instance==TIM3 && htim->Channel==HAL_TIM_ACTIVE_CHANNEL_1)
		pump_cbk=HAL_GetTick();
	if(htim->Instance==TIM9 && htim->Channel==HAL_TIM_ACTIVE_CHANNEL_1)
		fan1_cbk=HAL_GetTick();
	if(htim->Instance==TIM9 && htim->Channel==HAL_TIM_ACTIVE_CHANNEL_2)
		fan2_cbk=HAL_GetTick();	
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void HAL_CAN_RxCpltCallback(CAN_HandleTypeDef* hcan) {
	if(canBuffer)
		_buffer_push(canBuffer->rx,hcan->pRxMsg,sizeof(CanRxMsgTypeDef));
	HAL_CAN_Receive_IT(hcan, CAN_FIFO0);
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void HAL_CAN_TxCpltCallback(CAN_HandleTypeDef* hcan) {
	if(canBuffer)
		if(_buffer_pull(canBuffer->tx,hcan->pTxMsg,sizeof(CanTxMsgTypeDef)))
			HAL_CAN_Transmit_IT(hcan);
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void HAL_CAN_ErrorCallback(CAN_HandleTypeDef* hcan) {
		HAL_CAN_Receive_IT(hcan, CAN_FIFO0);
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void HAL_SYSTICK_Callback(void) {
		TIM10->CCR1=fan_drive;
		for(int i=0; i<__VALVES; ++i)
			if(valve_timeout[i] && HAL_GetTick() > valve_timeout[i]) {
				valve_timeout[i]=0;
				if(valve_drive[i])
					valve_drive[i]=0;
				else
					valve_drive[i]=__PWMRATE;
			}
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void	date_time(uint32_t d,uint32_t t) {
	int day=d % 32;
	int month=(d>>5) % 16;
	int year=(d>>9) + 2000;
	
	__print("%4d-%02d-%d%5d:%02d",day,month,year,t/3600,(t/60)%60);
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
__weak	void	Watchdog() {
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
int		ff_pack(int mode) {
int 	i,f,e,*p,*q,buf[SECTOR_SIZE/4];
int		c0=0,c1=0;

			f=FATFS_SECTOR;																															// f=koda prvega 128k sektorja
			e=PAGE_SIZE;																																// e=velikost sektorja
			p=(int *)FATFS_ADDRESS;																											// p=hw adresa sektorja
			do {
				do {
					++c0;
					Watchdog();																															//jk822iohfw
					q=&p[SECTOR_SIZE/4+1];																									
					while(p[SECTOR_SIZE/4] != q[SECTOR_SIZE/4] && q[SECTOR_SIZE/4] != -1)		// iskanje ze prepisanih sektorjev
						q=&q[SECTOR_SIZE/4+1];
					if(q[SECTOR_SIZE/4] == -1) {																						// ce ni kopija, se ga prepise na konec fs
						for(i=0; i<SECTOR_SIZE/4;++i)
							buf[i]=~p[i];
						Watchdog();
						if(mode)
							disk_write (0,(uint8_t *)buf,p[SECTOR_SIZE/4],1);										// STORAGE_Write bo po prvem brisanju zacel na
					} else																																	// zacetku !!!
						++c1;
					p=&p[SECTOR_SIZE/4+1]; 
				} while(((int)p)-FATFS_ADDRESS <  e && p[SECTOR_SIZE/4] != -1);						// prepisana cela stran...
				if(mode)
					FLASH_Erase(f,1);																												// brisi !
				f+=FLASH_SECTOR_1; 
				e+=PAGE_SIZE;
			} while(p[SECTOR_SIZE/4] != -1);	
			if(mode) {
				FLASH_Erase(f,1);																													// se zadnja !
				return 0;
			} else 
				return(100*c1/c0);
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void vApplicationMallocFailedHook( void ) {
	__print("memory error...");
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void vApplicationStackOverflowHook( TaskHandle_t xTask, signed char *pcTaskName ) {
	__print("stack error in...%s",pcTaskName);
}
