#ifndef _NIGHT_CORE_H_
#define _NIGHT_CORE_H_

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include <sys/stat.h>
#include <errno.h>
#include <stddef.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <pwd.h>
#include <grp.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <sys/utsname.h>

#define  NIGHT_OK				0
#define  NIGHT_ERROR			-1
#define  NIGHT_AGAIN			-2
#define  NIGHT_BUSY				-3
#define  NIGHT_DONE				-4
#define  NIGHT_DECLINED			-5
#define  NIGHT_ABORT			-6

#define LF 		((char)'\n')
#define CR 		((char)'\r')
#define CRLF	"\r\n"

#define night_align_ptr(p, a)                                                   	\
    	(char *) (((uintptr_t) (p) + ((uintptr_t) a - 1)) & ~((uintptr_t) a - 1))
    	
#define NIGHT_ALIGNMENT			sizeof(unsigned long)

#define NIGHT_MAXHOSTNAMELEN	(256)

#define NIGHT_PID_PATH			"pid/nginx.pid"
#define NIGHT_OLDPID_EXT     	".oldbin"

#define NIGHT_MAX_INT_T_VALUE	9223372036854775807

extern int		trace_file_fd;
extern pid_t	night_pid;
extern pid_t	night_parent;

extern int		night_dump_config;

extern
char			**night_os_argv;

extern
int				night_argc;

extern
char			**night_argv;

extern
char			**night_os_environ;
   
extern
size_t 			night_modules_n;

typedef struct night_cycle_s night_cycle_t;
    
#include "night_times.h"
#include "night_palloc.h"
#include "night_rbtree.h"
#include "night_string.h"
#include "night_array.h"
#include "night_file.h"
#include "night_buf.h"
#include "night_conf_file.h"
#include "night_list.h"
#include "night_open_file.h"
#include "night_shm_zone.h"
#include "night_listening.h"
#include "night_queue.h"
#include "night_conf.h"
#include "night_command.h"
#include "night_module.h"
#include "night_files.h"
#include "night_cycle.h"
#include "night_modules.h"
#include "night_string.h"
#include "night_slab.h"
#include "night_core_conf.h"
#include "night_core_module_ctx.h"
#include "night_hash_keys_arrays.h"
#include "night_hash.h"
#include "night_variable_value.h"
#include "night_event.h"
#include "night_http.h"

#include "night_os_config.h"

extern
night_cycle_t 	*night_cycle;

extern volatile
night_time_t	*night_cached_time;

extern
night_module_t	*night_modules[];

#endif /* _NIGHT_CORE_H_ */
