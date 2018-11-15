#ifndef		_CAN_H
#define		_CAN_H

#include	"stm32f4xx_hal.h"
#include	<string.h>
#include	"cli.h"
#include	"err.h"
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
class _CAN : public _TERM {
private:
	int		filter_count, ecTimeout, dlTimeout;
	int		dl_ch1, dl_ch2;
	_CLI	*remote;
	_err	error;
	static CAN_HandleTypeDef *hcan;

public:
  _CAN(CAN_HandleTypeDef *handle);
	virtual	void		Newline(void);
	virtual int			Fkey(int);
	virtual FRESULT	Decode(char *);

	void	pollRx(void *);
	void	Status(_err);
	_err	Status();

	void	canFilterCfg(int, int, int, int);
	int		SendRemote(int);
	static void	Send(int, void *, int);
	static _io  *io,*ioFsw;	
};
#endif
