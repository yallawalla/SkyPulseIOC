#ifndef		_PROC_H
#define		_PROC_H

#ifdef __cplusplus
 extern "C" {
#endif

#include	"io.h"
#define		_PROC_BUFFER_SIZE 16

typedef		void *func(void *);
 
typedef	struct {
func					*f;
void					*arg;
char					*name;
int						t,dt,to;
TaskHandle_t	*task;
} _proc;

void					_wait(int,void *(*)(void));
extern				_buffer 	*_proc_buf;
void					_proc_list(void),
							_proc_remove(void *,void *),
							*_proc_loop(void);
_proc					*_proc_add(void *,void *,char *,int),
							*_proc_find(void *,void *);

#ifdef __cplusplus
}
#endif

#endif
