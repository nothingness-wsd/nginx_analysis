
#ifndef _NIGHT_HTTP_CORE_MODULE_H_
#define _NIGHT_HTTP_CORE_MODULE_H_

#include "night_array.h"
#include "night_queue.h"
#include "night_hash.h"
#include "night_http_request.h"

#define NIGHT_LISTEN_BACKLOG (512)

#define night_get_main_conf(cf,module) \
        (((night_http_conf_t*)cf->ctx)->main_conf[module.ctx_index])
    
typedef struct night_http_server_name_s 		night_http_server_name_t;
typedef struct night_http_error_page_s 			night_http_error_page_t;
typedef struct night_http_location_queue_s 		night_http_location_queue_t;
typedef struct night_http_phase_engine_s 		night_http_phase_engine_t;
typedef struct night_http_phase_handler_s		night_http_phase_handler_t;
typedef struct night_http_phase_s				night_http_phase_t;

typedef int (*night_http_phase_handler_pt)(night_http_request_t *r, night_http_phase_handler_t *ph);

typedef enum 
{
    NIGHT_HTTP_POST_READ_PHASE = 0,

    NIGHT_HTTP_SERVER_REWRITE_PHASE,

    NIGHT_HTTP_FIND_CONFIG_PHASE,
    NIGHT_HTTP_REWRITE_PHASE,
    NIGHT_HTTP_POST_REWRITE_PHASE,

    NIGHT_HTTP_PREACCESS_PHASE,

    NIGHT_HTTP_ACCESS_PHASE,
    NIGHT_HTTP_POST_ACCESS_PHASE,

    NIGHT_HTTP_PRECONTENT_PHASE,

    NIGHT_HTTP_CONTENT_PHASE,

    NIGHT_HTTP_LOG_PHASE
} night_http_phases;


struct night_http_phase_s
{
    night_array_t                handlers;
};

struct night_http_phase_handler_s
{
    night_http_phase_handler_pt  			checker;
    night_http_handler_pt        			handler;
    int                 					next;
};

// 驱动 HTTP 请求在各个处理阶段（phases）中的执行流程。它是 Nginx 阶段化（phased）请求处理架构的“执行引擎”
// 1. 统一阶段执行入口
// 2. 解耦“阶段逻辑”与“模块实现”, 每个模块只需在配置阶段向特定 phase 注册自己的 handler
struct night_http_phase_engine_s
{
	// handlers 是一个 night_http_phase_handler_t 数组
	// 这个数组按处理顺序排列了所有阶段的 handler（包括 rewrite、access、content 等），每个 handler 包含一个 checker 函数指针和一个 handler 函数指针
	// checker 是实际被调用的“调度器”，它决定如何调用 handler 并处理返回值。
    night_http_phase_handler_t  			*handlers;
    int                 					server_rewrite_index;
    int                 					location_rewrite_index;
};

struct night_http_location_queue_s
{
    night_queue_t 						queue;
    night_str_t							*name;
    night_http_core_loc_conf_t 			*exact;
    night_http_core_loc_conf_t 			*inclusive;
    night_queue_t 						list;
    char 								*file_name;
    int 								line;
};

struct night_http_error_page_s
{
    int 		status;
    night_str_t value;
};

struct night_http_server_name_s
{
    night_http_core_srv_conf_t 	*server;
    night_str_t 				name;
};

struct night_http_core_main_conf_s
{
    night_array_t 				servers;
    night_array_t 				ports;
    
    night_http_phase_engine_t	phase_engine;
    
    night_hash_t				headers_in_hash;
    
    night_http_phase_t			phases[NIGHT_HTTP_LOG_PHASE + 1];
    //night_array_t variables;
};

struct night_http_core_srv_conf_s
{
	night_http_conf_t 			*ctx;
	night_http_server_name_t 	server_name;
	
	int64_t 					client_header_timeout;
	size_t 						client_header_buffer_size;
};

struct night_http_core_loc_conf_s
{
	night_str_t 						name;
	int 								exact_match;
	
	night_queue_t						*locations;
	void								**loc_conf;
	
	night_str_t 						root;
	night_array_t  						*root_lengths;
	night_array_t 						error_pages;
	night_http_location_tree_node_t 	*static_locations_tree;
	
	night_open_file_cache_t				*open_file_cache;
	
	night_hash_t    					types_hash;
	
	size_t    	    					types_hash_max_size;
    size_t    							types_hash_bucket_size;
    
    night_array_t  						*types;
    
    night_str_t     					default_type;
    
    uint64_t    						keepalive_timeout;
	
	unsigned    						internal:1;              
};

void*
night_http_create_main_conf(night_conf_t *cf);

void*
night_http_create_srv_conf(night_conf_t *cf);

void*
night_http_create_loc_conf(night_conf_t *cf);

int
night_http_server(night_conf_t	*cf, night_command_t *cmd, void *conf);

int
night_http_listen(night_conf_t *cf, night_command_t *cmd, void *conf);

int
night_http_add_listen(night_conf_t *cf, night_http_core_srv_conf_t *sc, night_http_listen_opt_t *lsopt);

in_port_t
night_get_port(struct sockaddr *sa);

int
night_http_server_name(night_conf_t *cf, night_command_t *cmd, void *conf);

int
night_http_location(night_conf_t *cf, night_command_t *cmd, void *conf);

int
night_http_add_location(night_conf_t *cf, night_queue_t **locations, night_http_core_loc_conf_t *lc);

int
night_http_root(night_conf_t *cf, night_command_t *cmd, void *conf);

int
night_http_error_page(night_conf_t *cf, night_command_t *cmd, void *conf);

void
night_http_handler(night_http_request_t *r);

void
night_http_core_run_phases(night_http_request_t *r);

int
night_http_core_rewrite_phase(night_http_request_t *r, night_http_phase_handler_t *ph);

int
night_http_core_find_config_phase(night_http_request_t *r, night_http_phase_handler_t *ph);

int
night_http_core_post_rewrite_phase(night_http_request_t *r, night_http_phase_handler_t *ph);

int
night_http_core_access_phase(night_http_request_t *r, night_http_phase_handler_t *ph);

int
night_http_core_post_access_phase(night_http_request_t *r, night_http_phase_handler_t *ph);

int
night_http_core_content_phase(night_http_request_t *r, night_http_phase_handler_t *ph);

int
night_http_core_generic_phase(night_http_request_t *r, night_http_phase_handler_t *ph);

int
night_http_core_find_location(night_http_request_t *r);

int
night_http_core_find_static_location(night_http_request_t *r, night_http_location_tree_node_t *node);

void
night_http_update_location_config(night_http_request_t *r);

char*
night_http_map_uri_to_path(night_http_request_t *r, night_str_t *path, size_t *root_length, size_t reserved);

int
night_http_internal_redirect(night_http_request_t *r, night_str_t *uri, night_str_t *args);

void
night_http_set_exten(night_http_request_t *r);

int
night_http_set_etag(night_http_request_t *r);

int
night_http_set_content_type(night_http_request_t *r);

int
night_http_send_header(night_http_request_t *r);

int
night_http_output_filter(night_http_request_t *r, night_chain_t *in);

#endif /* _NIGHT_HTTP_CORE_MODULE_H_ */
