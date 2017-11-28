#include "proc.h"
SemaphoreHandle_t _sWait=NULL;
TaskHandle_t 			_tWait[]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
TaskHandle_t 			*tWait=_tWait;
_buffer						*_proc_buf=NULL;
//______________________________________________________________________________
void 	_proc_code(void *arg) {
_proc *p=(_proc *)arg;
			while(1) {
				if(HAL_GetTick() >= p->t) {
					p->to = HAL_GetTick() - p->t;
					p->f(p->arg);
					p->t = HAL_GetTick() + p->dt;
				}
			}
}
//______________________________________________________________________________
_proc	*_proc_add(void *f,void *arg,char *name, int dt) {
_proc	*p=malloc(sizeof(_proc));
			if(p != NULL) {
				p->f=(func *)f;
				p->arg=arg;
				p->name=name;
				p->t=HAL_GetTick();
				p->dt=dt;
				if(!_proc_buf) {
					_proc_buf=_buffer_init(_PROC_BUFFER_SIZE*sizeof(_proc));				
				}
				_buffer_push(_proc_buf,&p,sizeof(_proc *));
			}
			return p;
}
//______________________________________________________________________________
void	*_proc_loop(void) {
			_proc	*p=NULL;
			if(_proc_buf && _buffer_pull(_proc_buf,&p,sizeof(_proc *)) && p) {
				if(HAL_GetTick() >= p->t) {
					p->to = HAL_GetTick() - p->t;
					p->f(p->arg);
					p->t = HAL_GetTick() + p->dt;
				}
				_buffer_push(_proc_buf,&p,sizeof(_proc *));
			}
			return p;
}
//______________________________________________________________________________
void	_proc_remove(void  *f,void *arg) {
			_proc	*p;
int		i=_buffer_count(_proc_buf)/sizeof(_proc *);
			while(i--) {
				_buffer_pull(_proc_buf,&p,sizeof(_proc *));
				if(f == p->f && arg == p->arg)
					free(p);
				else
					_buffer_push(_proc_buf,&p,sizeof(_proc *));
			}
}
//______________________________________________________________________________
_proc	*_proc_find(void  *f,void *arg) {
_proc	*p,*q=NULL;
int		i=_buffer_count(_proc_buf)/sizeof(_proc *);
			while(i--) {
				_buffer_pull(_proc_buf,&p,sizeof(_proc *));
				if(f == p->f && (arg == p->arg || !arg))
					q=p;
				_buffer_push(_proc_buf,&p,sizeof(_proc *));
			}
			return q;
}
//______________________________________________________________________________
void	_proc_list(void) {
_proc	*p;	
int		i	=_buffer_count(_proc_buf)/sizeof(_proc *);
			_print("...\r\n");
			while(i--) {
				_buffer_pull(_proc_buf,&p,sizeof(_proc *));
				_print("%08X,%08X,%s,%d\r\n",(int)p->f,(int)p->arg,p->name,p->to);
				_buffer_push(_proc_buf,&p,sizeof(_proc *));
			}
}
//______________________________________________________________________________
void	_p_loop(void) {
			if(_sWait==NULL)
				_sWait=xSemaphoreCreateMutex();
			if(xSemaphoreTake(_sWait,portMAX_DELAY)) {
				_proc	*p=NULL;
				if(_proc_buf && _buffer_pull(_proc_buf,&p,sizeof(_proc *)) && p) {
					if(HAL_GetTick() >= p->t) {
						p->to = HAL_GetTick() - p->t;
						p->f(p->arg);
						p->t = HAL_GetTick() + p->dt;
					}
					_buffer_push(_proc_buf,&p,sizeof(_proc *));
				}
				xSemaphoreGive(_sWait);
			}
			taskYIELD();
}
//___________________________________________________________________________
void	_task(const void *t) {
			while(1) {
				_p_loop();
				vTaskDelay(1);
			}
}
//___________________________________________________________________________
void	_wait(int t) {
			_io *temp=_stdio(NULL);
			if(*tWait==NULL)
				xTaskCreate((TaskFunction_t)_task, "---", 512, NULL, 0, tWait);
			++tWait;
			xSemaphoreGive(_sWait);
			taskYIELD();
			vTaskDelay(t);
			xSemaphoreTake(_sWait,portMAX_DELAY);
			--tWait;
			_stdio(temp);
}
