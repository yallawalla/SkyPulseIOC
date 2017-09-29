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
  _CAN(CAN_HandleTypeDef *);
	_CLI	*remote;
	int		SendRemote(void);
	int		filter_count;
	CAN_HandleTypeDef *hcan;

	
public:
	virtual void			Newline(void);
	virtual FRESULT		Decode(char *);
	virtual int				Fkey(int);
	static _CAN*			instance;

	_io								*io,*canBuffer;	
	void 							*Task(void *),
										canFilterCfg(int, int),
										Send(CanTxMsgTypeDef *);
	
	
	static _CAN				*InstanceOf(CAN_HandleTypeDef *hcan) {
										if(instance==NULL)
											instance=new _CAN(hcan);
										return instance;
	}
};
#endif
