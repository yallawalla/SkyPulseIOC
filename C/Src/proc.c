#include "proc.h"
_buffer				*_proc_buf=NULL;
SemaphoreHandle_t sobj=NULL;
//______________________________________________________________________________
void 	_proc_code(void *arg) {
_proc *p=(_proc *)arg;
			while(1) {
				if(HAL_GetTick() >= p->t) {
					if(xSemaphoreTake(sobj,5)) {
						p->to = HAL_GetTick() - p->t;
						p->f(p->arg);
						xSemaphoreGive(sobj);
						p->t = HAL_GetTick() + p->dt;
					}
				}
				vTaskDelay(1);	
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
					sobj = xSemaphoreCreateMutex();					
				}
				_buffer_push(_proc_buf,&p,sizeof(_proc *));
				xTaskCreate(_proc_code,name,512,p,0,p->task);
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
			__print("...\r\n");
			while(i--) {
				_buffer_pull(_proc_buf,&p,sizeof(_proc *));
				__print("%08X,%08X,%s,%d\r\n",(int)p->f,(int)p->arg,p->name,p->to);
				_buffer_push(_proc_buf,&p,sizeof(_proc *));
			}
}
//___________________________________________________________________________
void	_wait(int t,void *(*f)(void)) {
//int		to=HAL_GetTick()+t;
//			while(to > HAL_GetTick()) {
//				if(f)
//					f();
//			}
			xSemaphoreGive(sobj);
			vTaskDelay(t);
			while(!xSemaphoreTake(sobj,5))
				osDelay(1);
}

