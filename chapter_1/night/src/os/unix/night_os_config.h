#ifndef _NIGHT_LINUX_CONFIG_H_
#define _NIGHT_LINUX_CONFIG_H_

#include "night_atomic.h"
#include "night_time.h"
#include "night_alloc.h"
#include "night_posix_init.h"

typedef struct night_os_io_s night_os_io_t;

struct night_os_io_s
{

};

extern
size_t  			night_pagesize;

extern
char				night_linux_kern_ostype[50];

extern
char  				night_linux_kern_osrelease[50];

extern
night_os_io_t  		night_os_io;

extern
size_t				night_cacheline_size;

extern
size_t				night_pagesize_shift;

extern
size_t				night_ncpu;

#include "night_linux_init.h"
#include "night_setproctitle.h"

#endif /* _NIGHT_LINUX_CONFIG_H_ */
