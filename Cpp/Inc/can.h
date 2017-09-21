#ifndef		_CAN_H
#define		_CAN_H

#include	"stm32f4xx_hal.h"
#include	<string.h>
#include	"fs.h"
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
class _CAN : public _TERM {

private:
	CAN_HandleTypeDef *hcan;
	_FS *	remote;
	int		SendRemote(void);
	int		filter_count;

	
public:
  _CAN(CAN_HandleTypeDef *);
	virtual void			Newline(void);
	virtual FRESULT		Decode(char *);
	virtual int				Fkey(int);
	static _CAN*			instance;

	_io								*canBuffer;	
	void 							Task(void *),
										canFilterCfg(int, int),
										Send(CanTxMsgTypeDef *);
	
	
	static _CAN				*InstanceOf(CAN_HandleTypeDef *hcan) {
		if(instance==NULL)
			instance=new _CAN(hcan);
		return instance;
	}
};
#endif
