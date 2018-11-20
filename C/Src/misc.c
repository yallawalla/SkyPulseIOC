#include	"stm32f4xx_hal.h"
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
	_print("\r\n:02000004%04X%02X\r\n",(a>>16),-(2+4+((a>>16)/256)+((a>>16) % 256)) & 255);
	while(n) {
		int	sum = _MAXBYTESLINE+(a & 0xffff)/256+(a & 0xff);
		_print(":%02X%04X00",_MAXBYTESLINE,(a & 0xffff));
		for(k = 0; k<_MAXBYTESLINE; ++k) {
			_print("%02X",(*(unsigned char *)a & 0xff));
			sum += *(unsigned char *)a;
			if(((++a) & 0xffff) == 0 || --n == 0)
				break;
		}
		_print("%02X\r\n",-sum & 0xff);
		if(n && (a & 0xffff) == 0) {
			_print(":02000004%04X,%02X\r\n",(a>>16),-(2+4+((a>>16)/256)+((a>>16) % 256)) & 255);
		}
	}
	_print(":00000001FF\r\n");
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void	pollUsart(_io *io) {
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
_io* ioUsart(UART_HandleTypeDef *huart, int sizeRx, int sizeTx) {
	_io* io=_io_init(sizeRx,sizeTx);
	if(io && huart) {
		io->huart=huart;
		HAL_UART_Receive_DMA(huart,(uint8_t*)io->rx->_buf,io->tx->size);
		_proc_add(pollUsart,io,"uart",0);
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
	if(htim->Instance==TIM3 && htim->Channel==HAL_TIM_ACTIVE_CHANNEL_1) {
		if(pumpTacho++ % 50 == 0)
			__BLUE1(50);
	}
	if(htim->Instance==TIM3 && htim->Channel==HAL_TIM_ACTIVE_CHANNEL_2) {
		if(flowTacho++ % 200 == 0)
			__GREEN1(50);
	}
	if(htim->Instance==TIM9 && htim->Channel==HAL_TIM_ACTIVE_CHANNEL_1) {
		if(fanTacho++ % 50 == 0)
			__YELLOW1(50);
	}
	if(htim->Instance==TIM9 && htim->Channel==HAL_TIM_ACTIVE_CHANNEL_2)
		if(flowTacho++ % 200 == 0)
			__GREEN1(50);
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef* hcan) {
	CAN_RxHeaderTypeDef hdr;
	uint8_t							data[8];
	HAL_CAN_GetRxMessage(hcan,CAN_RX_FIFO0,&hdr,data);
	_buffer_push(canBuffer->rx,&hdr,sizeof(CAN_RxHeaderTypeDef));
	_buffer_push(canBuffer->rx,data,hdr.DLC);
}
//
void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef* hcan) {
	HAL_CAN_RxFifo0MsgPendingCallback(hcan);
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef* hcan) {
	CAN_TxHeaderTypeDef hdr;
	uint8_t							data[8];
	uint32_t						mailbox;
	if(_buffer_pull(canBuffer->tx,&hdr,sizeof(CAN_TxHeaderTypeDef))) {
		_buffer_pull(canBuffer->tx,data,hdr.DLC*sizeof(uint8_t));
		HAL_CAN_AddTxMessage(hcan, &hdr, (uint8_t *)data, &mailbox);
	}
}
//
void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef* hcan) {
	HAL_CAN_TxMailbox2CompleteCallback(hcan); 
}
//
void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef* hcan) {
	HAL_CAN_TxMailbox2CompleteCallback(hcan); 
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void HAL_CAN_ErrorCallback(CAN_HandleTypeDef* hcan) {
		hcan->Instance->MSR |= 0x0004;
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
	
	_print("%4d-%02d-%d%5d:%02d",day,month,year,t/3600,(t/60)%60);
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void	Watchdog() {
			HAL_IWDG_Refresh(&hiwdg);	
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void	Watchdog_init(int t) {
			hiwdg.Instance = IWDG;
			hiwdg.Init.Prescaler = IWDG_PRESCALER_32;
			hiwdg.Init.Reload = t;
			HAL_IWDG_Init(&hiwdg);
}
/*******************************************************************************
* Function Name	: ff_pack
* Description		: packing flash dile system sectors
* Input					: mode flag, 0-analyze, 1-rewrite
* Output				: 
* Return				: percentage of number of sectors reduced
*******************************************************************************/
int		ff_pack(int mode) {
int 	i,f,e,*p,*q,buf[SECTOR_SIZE/4];
int		c0=0,c1=0;

			Watchdog_init(4000);
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
				if(mode) {
					_print(".");
					_wait(2);
					FLASH_Erase(f,1);																												// brisi !
				}
				f+=FLASH_SECTOR_1; 
				e+=PAGE_SIZE;
			} while(p[SECTOR_SIZE/4] != -1);	
			if(mode) {
				_print(". OK");
				_wait(2);
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
	_print("memory error...");
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void vApplicationStackOverflowHook( TaskHandle_t xTask, signed char *pcTaskName ) {
	_print("stack error in...%s",pcTaskName);
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
struct {
		uint32_t			timeout[8];
		GPIO_TypeDef*	gpio[8]; 
		uint32_t			pin[8];	
} leds = {
	{0,0,0,0,0,0,0,0},
	{GPIOD,GPIOD,GPIOD,GPIOD,GPIOD,GPIOD,GPIOD,GPIOD},
	{GPIO_PIN_0,GPIO_PIN_1,GPIO_PIN_2,GPIO_PIN_3,GPIO_PIN_4,GPIO_PIN_5,GPIO_PIN_6,GPIO_PIN_7}
};
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void	led_poll(void) {
	for(int i = 0; i < sizeof(leds.pin)/sizeof(uint32_t); ++i)
		if(HAL_GetTick() < leds.timeout[i])	
			HAL_GPIO_WritePin(leds.gpio[i],leds.pin[i], GPIO_PIN_RESET);
		else
			HAL_GPIO_WritePin(leds.gpio[i],leds.pin[i], GPIO_PIN_SET);
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
char *trim(char **c) {
	if(!c)
		return NULL;
	if(*c) {
		while(**c==' ') ++*c;
		for(char *cc=strchr(*c,0); *c != cc && *--cc==' '; *cc=0);
	}
	return *c;
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void __RED1(int32_t t)			{ leds.timeout[0]=t+HAL_GetTick(); };
void __GREEN1(int32_t t)		{ leds.timeout[1]=t+HAL_GetTick(); };
void __YELLOW1(int32_t t)		{ leds.timeout[2]=t+HAL_GetTick(); };
void __BLUE1(int32_t t)			{ leds.timeout[3]=t+HAL_GetTick(); };
void __RED2(int32_t t)			{ leds.timeout[4]=t+HAL_GetTick(); };
void __GREEN2(int32_t t)		{ leds.timeout[5]=t+HAL_GetTick(); };
void __YELLOW2(int32_t t) 	{ leds.timeout[6]=t+HAL_GetTick(); };
void __BLUE2(int32_t t)			{ leds.timeout[7]=t+HAL_GetTick(); };
char* Days[]							= { "Mon","Tue","Wed","Thu","Fri","Sat","Sun" };
char* Months[]						= { "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec" };
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
****************************f***************************************************/
void	printVersion() {
			_print(" %d.%02d %s <%08X>",
				SW_version/100,SW_version%100,
					__DATE__,
						HAL_CRC_Calculate(&hcrc,__Vectors, (FATFS_ADDRESS-(int)__Vectors)/sizeof(int)));
}
