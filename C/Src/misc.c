#include	"stm32f4xx_hal.h"
#include 	"io.h"
#include 	"proc.h"
#include 	"misc.h"
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
	printf("\r\n:02000004%04X%02X\r\n",(a>>16),-(2+4+((a>>16)/256)+((a>>16) % 256)) & 255);
	while(n) {
		int	sum = _MAXBYTESLINE+(a & 0xffff)/256+(a & 0xff);
		printf(":%02X%04X00",_MAXBYTESLINE,(a & 0xffff));
		for(k = 0; k<_MAXBYTESLINE; ++k) {
			printf("%02X",(*(unsigned char *)a & 0xff));
			sum += *(unsigned char *)a;
			if(((++a) & 0xffff) == 0 || --n == 0)
				break;
		}
		printf("%02X\r\n",-sum & 0xff);
		if(n && (a & 0xffff) == 0) {
			printf(":02000004%04X,%02X\r\n",(a>>16),-(2+4+((a>>16)/256)+((a>>16) % 256)) & 255);
		}
	}
	printf(":00000001FF\r\n");
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
uint32_t pump_cbk=0;
uint32_t fan1_cbk=0;
uint32_t fan2_cbk=0;
uint16_t pump_drive,fan_drive;
uint16_t valve_drive[4]={0,0,0,0};
uint32_t valve_time[4]={0,0,0,0};


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
void HAL_SYSTICK_Callback(void) {
		TIM10->CCR1=fan_drive;
		for(int i=0; i<sizeof(valve_time)/sizeof(uint32_t); ++i)
			if(valve_time[i] && HAL_GetTick() > valve_time[i]) {
				valve_time[i]=0;
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
	
	printf("%4d-%d-%d%5d:%02d",day,month,year,t/3600,(t/60)%60);
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void SetTimeDate() {
extern RTC_HandleTypeDef hrtc;
RTC_DateTypeDef sDate;
	int d,y;
	char month[4];
	char *months[]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

	sscanf(__DATE__,"%s %d %d",month,&d,&y);
	for(int i=0; i<6;++i)
		if(!strcmp(months[i],month))
			break;
		
		
	
  sDate.WeekDay = i;
  sDate.Month = m;
  sDate.Date = 0x24;
  sDate.Year = 0x17;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
 //   _Error_Handler(__FILE__, __LINE__);
  }

    HAL_RTCEx_BKUPWrite(&hrtc,RTC_BKP_DR0,0x32F2);
//  }
}
