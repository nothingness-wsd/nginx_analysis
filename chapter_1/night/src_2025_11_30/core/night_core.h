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

#define NIGHT_OK    			(0)
#define NIGHT_ERROR 			(-1)
#define NIGHT_AGAIN 			(-2)
#define NIGHT_DECLINED 			(-3)
#define NIGHT_DONE      		(-4)

#define NIGHT_PROCESS_MASTER 	(1)
#define NIGHT_PROCESS_SINGLE	(2)
#define NIGHT_PROCESS_WORKER 	(3)
#define NIGHT_PROCESS_SIGNALLER (4)

#define LF 		((char)'\n')
#define CR 		((char)'\r')
#define CRLF	"\r\n"

#define NIGHT_ALIGN (8)

#define night_align(p, a)	(((p) + (a - 1)) & ~(a - 1))
#define night_align_ptr(p, a)													\
		(char*) (((uintptr_t) (p) + ((uintptr_t) a - 1)) & ~((uintptr_t) a - 1))

#define NIGHT_HOST_NAME_LEN (128)

#define NIGHT_CONF_FILE "conf/night.conf"

#define night_null_string {0,NULL}
#define night_string(str) {sizeof(str) - 1, (char*) str }

#define night_min(v1,v2) ((v1>v2) ? (v2) : (v1)) 

#define NIGHT_INT64_STRLEN (sizeof("-9223372036854775808") - 1)

#define night_abs(value) (((value) >= 0) ? (value) : -(value))

#define NIGHT_READ_EVENT 	EPOLLIN
#define NIGHT_WRITE_EVENT 	EPOLLOUT

#define NIGHT_PIDFILE 	"pid/night.pid"

#define NIGHT_INVALID_FD (-1)

#define NIGHT_MAX_OFF_T_VALUE	9223372036854775807

#define NIGHT_MAX_TIME_T_VALUE	9223372036854775807

#define NIGHT_OFF_T_LEN  		(sizeof("-9223372036854775808") - 1)
#define NIGHT_TIME_T_LEN  		(sizeof("-9223372036854775808") - 1)
	

typedef struct night_module_s 					night_module_t;
typedef struct night_conf_s						night_conf_t;
typedef struct night_str_s 						night_str_t;
typedef struct night_cycle_s 					night_cycle_t;
typedef struct night_pool_s 					night_pool_t;
typedef struct night_conf_file_s 				night_conf_file_t;
typedef struct night_command_s 					night_command_t;
typedef struct night_http_conf_s 				night_http_conf_t;
typedef struct night_queue_s 					night_queue_t;
typedef struct night_http_core_main_conf_s 		night_http_core_main_conf_t;
typedef struct night_http_core_srv_conf_s 		night_http_core_srv_conf_t;
typedef struct night_http_core_loc_conf_s		night_http_core_loc_conf_t;
typedef struct night_http_location_tree_node_s 	night_http_location_tree_node_t;
typedef struct night_http_listen_opt_s 			night_http_listen_opt_t;
typedef struct night_http_port_s 				night_http_port_t;
typedef struct night_listening_s 				night_listening_t;
typedef struct night_buf_s 						night_buf_t;
typedef struct night_connection_s 				night_connection_t;
typedef struct night_event_s 					night_event_t;
typedef struct night_event_actions_s 			night_event_actions_t;
typedef struct night_time_s 					night_time_t;
typedef struct night_process_s 					night_process_t;
typedef struct night_channel_s 					night_channel_t;
typedef struct night_http_connection_s 			night_http_connection_t;
typedef struct night_http_request_s 			night_http_request_t;
typedef struct night_table_elt_s  				night_table_elt_t;
typedef struct night_open_file_info_s 			night_open_file_info_t;
typedef struct night_open_file_cache_s 			night_open_file_cache_t;
typedef struct night_file_s 					night_file_t;
typedef struct night_conf_s 					night_conf_t;
typedef struct night_chain_s           			night_chain_t;

typedef void (*night_event_handler_pt)(night_event_t *event);

//typedef struct night_variable_value_s 	night_variable_value_t;

struct night_str_s
{
	size_t 	len;
	char 	*data;
};

struct night_variable_value_s 
{
    size_t    	len:28;

    unsigned    valid:1;
    unsigned    no_cacheable:1;
    unsigned    not_found:1;
    unsigned    escape:1;

    char     	*data;
};

extern char 					**environ;

extern int 						trace_file_fd;
extern int 						error_log_fd;

extern int 						night_argc;
extern char						**night_argv;

extern int 						night_pid;
extern int 						night_parent_pid;

extern int 						night_modules_n;

extern night_cycle_t 			*night_cycle;

extern night_module_t*			night_modules[];

extern night_module_t 			night_core_module;
extern night_module_t			night_events_module;

extern night_module_t			night_event_core_module;
	
extern night_module_t 			night_http_module;

extern night_module_t 			night_http_core_module;

extern night_module_t 			night_http_index_module;

extern night_module_t 			night_http_rewrite_module;

extern night_module_t			night_http_limit_conn_module;

extern night_module_t			night_http_limit_req_module;

extern night_module_t			night_http_access_module;

extern night_module_t			night_http_auth_basic_module;

extern night_module_t			night_http_try_files_module;

extern night_module_t			night_http_mirror_module;

extern night_module_t			night_http_static_module;

extern night_module_t 			night_http_autoindex_module;

extern night_module_t			night_http_header_filter_module;

extern night_module_t			night_http_range_header_filter_module;
extern night_module_t			night_http_range_body_filter_module;

extern night_module_t			night_http_write_filter_module;

extern night_module_t			night_http_postpone_filter_module;
extern night_module_t			night_http_copy_filter_module;

extern night_module_t			night_http_log_module;

extern night_module_t 			night_epoll_module;

extern int 						night_http_modules_n;

extern night_str_t 				night_work_directory;

extern int						night_worker;

extern night_event_actions_t 	night_event_actions;

extern int						night_process;

extern int						night_exiting;

extern night_time_t				*night_cached_time;

extern int 						night_last_process;

extern int 						night_channel;

extern int						night_terminate;
extern int						night_quit;

extern int 						night_reap;
extern int 						night_reopen;

extern volatile uint64_t		night_current_msec;

extern char						*night_argv_last;

extern int						night_page_size;

#endif /* _NIGHT_CORE_H_ */
