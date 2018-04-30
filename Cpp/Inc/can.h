#ifndef		_CAN_H
#define		_CAN_H

#include	"stm32f4xx_hal.h"
#include	<string.h>
#include	"cli.h"
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
class _CAN : public _TERM {
private:
	int		filter_count,timeout;
	static CAN_HandleTypeDef *hcan;
	_CLI	*remote;

public:
  _CAN(CAN_HandleTypeDef *handle);
	virtual	void		Newline(void);
	virtual int			Fkey(int);
	virtual FRESULT	Decode(char *);

	void	pollRx(void *),
				canFilterCfg(int, int),
				Send(CanTxMsgTypeDef *);	
	int		SendRemote(int);
	static void	Send(int, void *, int);
	static _io  *io,*ioFsw;	
};
#endif
