#ifndef		_MISC_H
#define		_MISC_H

#ifdef __cplusplus
 extern "C" {
#endif

#include 	"stm32f4xx_hal.h"
#include 	"io.h"
HAL_StatusTypeDef	FLASH_Program(uint32_t, uint32_t);
HAL_StatusTypeDef	FLASH_Erase(uint32_t, uint32_t);
void	poll_uart(_io *);
_io* init_uart(UART_HandleTypeDef *, int, int);
	 
#define	FATFS_ADDRESS 0x08040000
#define	PAGE_SIZE			0x40000
#define	PAGE_COUNT		3
#define	SECTOR_SIZE		512
#define	CLUSTER_SIZE	4*SECTOR_SIZE
#define	SECTOR_COUNT	(int)(PAGE_SIZE*PAGE_COUNT/(SECTOR_SIZE + sizeof(uint32_t)))

void	dumpHex(int, int);
void	flushVCP(const void *);
void	flushUART(const void *);
HAL_StatusTypeDef canFilterCfg(CAN_HandleTypeDef *);

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;
extern CAN_HandleTypeDef	hcan2;
extern RTC_HandleTypeDef 	hrtc;
extern ADC_HandleTypeDef	hadc1;
extern TIM_HandleTypeDef	htim4;

extern	uint32_t pump_cbk, fan1_cbk, fan2_cbk,valve_timeout[];
extern	uint16_t pump_drive, fan_drive, valve_drive[], led_drive[];

extern _io	*_VCP,*canBuffer;
extern void	*CDC_Poll_FS(void *);

extern void	date_time(uint32_t, uint32_t);
extern void SetTimeDate(void);
extern	RTC_TimeTypeDef sTime;
extern	RTC_DateTypeDef sDate;

#ifdef __cplusplus
}
#endif

#endif
