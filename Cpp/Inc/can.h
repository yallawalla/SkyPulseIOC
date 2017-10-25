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
	int		SendRemote(void);
	int		filter_count;
	CAN_HandleTypeDef *hcan;
	_CLI	*remote;

public:
  _CAN(CAN_HandleTypeDef *handle);
	virtual	void		Newline(void);
	virtual int			Fkey(int);
	virtual FRESULT	Decode(char *);

	_io		*io;	
	void	Poll(void),
				canFilterCfg(int, int),
				Send(CanTxMsgTypeDef *);
				
	static void	task(_CAN *me) {
		me->Poll();
	}
};
#endif
