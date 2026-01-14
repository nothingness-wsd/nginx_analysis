#ifndef _NIGHT_HTTP_REQUEST_H_
#define _NIGHT_HTTP_REQUEST_H_

#include "night_list.h"

#define NIGHT_HTTP_UNKNOWN                   	0x00000001
#define NIGHT_HTTP_GET                       	0x00000002
#define NIGHT_HTTP_HEAD                      	0x00000004
#define NIGHT_HTTP_POST                     	0x00000008
#define NIGHT_HTTP_PUT                      	0x00000010
#define NIGHT_HTTP_DELETE                  	  	0x00000020
#define NIGHT_HTTP_MKCOL                     	0x00000040
#define NIGHT_HTTP_COPY                      	0x00000080
#define NIGHT_HTTP_MOVE                      	0x00000100
#define NIGHT_HTTP_OPTIONS                   	0x00000200
#define NIGHT_HTTP_PROPFIND                  	0x00000400
#define NIGHT_HTTP_PROPPATCH                 	0x00000800
#define NIGHT_HTTP_LOCK                      	0x00001000
#define NIGHT_HTTP_UNLOCK                    	0x00002000
#define NIGHT_HTTP_PATCH                     	0x00004000
#define NIGHT_HTTP_TRACE                     	0x00008000
#define NIGHT_HTTP_CONNECT                   	0x00010000

#define NIGHT_HTTP_VERSION_09                	9
#define NIGHT_HTTP_VERSION_10                	1000
#define NIGHT_HTTP_VERSION_11                	1001
#define NIGHT_HTTP_VERSION_20                	2000

#define NIGHT_HTTP_SWITCHING_PROTOCOLS       	101

#define NIGHT_HTTP_OK                        	200
#define NIGHT_HTTP_NO_CONTENT                	204
#define NIGHT_HTTP_PARTIAL_CONTENT           	206

#define NIGHT_HTTP_NOT_MODIFIED              	304

#define NIGHT_HTTP_BAD_REQUEST					400
#define NIGHT_HTTP_NOT_FOUND                 	404
#define NIGHT_HTTP_NOT_ALLOWED					405
#define NIGHT_HTTP_REQUEST_TIME_OUT          	408
#define NIGHT_HTTP_REQUEST_URI_TOO_LARGE		414

#define NIGHT_HTTP_INTERNAL_SERVER_ERROR     	500
#define NIGHT_HTTP_NOT_IMPLEMENTED           	501
#define NIGHT_HTTP_VERSION_NOT_SUPPORTED     	505

#define NIGHT_HTTP_PARSE_HEADER_DONE         	1

#define NIGHT_HTTP_PARSE_INVALID_METHOD			10
#define NIGHT_HTTP_PARSE_INVALID_REQUEST     	11
#define NIGHT_HTTP_PARSE_INVALID_VERSION     	12
#define NIGHT_HTTP_PARSE_INVALID_09_METHOD		13
#define NIGHT_HTTP_PARSE_INVALID_HEADER      	14

#define NIGHT_HTTP_CONNECTION_CLOSE          	1
#define NIGHT_HTTP_CONNECTION_KEEP_ALIVE     	2

// must be 2^n
#define NIGHT_HTTP_LC_HEADER_LEN             	32

#define NIGHT_HTTP_MAX_URI_CHANGES           	10

typedef struct night_http_headers_out_s 		night_http_headers_out_t;
typedef struct night_http_headers_in_s 			night_http_headers_in_t;
typedef struct night_http_header_s				night_http_header_t;

typedef int (*night_http_header_handler_pt)(night_http_request_t *r, night_table_elt_t *h, size_t offset);

typedef int (*night_http_handler_pt)(night_http_request_t *r);
typedef void (*night_http_event_handler_pt)(night_http_request_t *r);

typedef struct night_http_posted_request_s  night_http_posted_request_t;

typedef enum 
{
    NIGHT_HTTP_INITING_REQUEST_STATE = 0,
    NIGHT_HTTP_READING_REQUEST_STATE,
    NIGHT_HTTP_PROCESS_REQUEST_STATE,

    NIGHT_HTTP_CONNECT_UPSTREAM_STATE,
    NIGHT_HTTP_WRITING_UPSTREAM_STATE,
    NIGHT_HTTP_READING_UPSTREAM_STATE,

    NIGHT_HTTP_WRITING_REQUEST_STATE,
    NIGHT_HTTP_LINGERING_CLOSE_STATE,
    NIGHT_HTTP_KEEPALIVE_STATE
} night_http_state_e;

struct night_http_posted_request_s 
{
    night_http_request_t               *request;
    night_http_posted_request_t        *next;
};

struct night_http_header_s
{
    night_str_t                         name;
    size_t                        		offset;
    night_http_header_handler_pt        handler;
};

struct night_http_headers_in_s
{
	night_str_t                         		server;
	
	night_list_t								headers;
	
	night_table_elt_t                  			*host;
	night_table_elt_t	                  		*connection;
	night_table_elt_t                  			*user_agent;
	night_table_elt_t                  			*accept;
	night_table_elt_t                  			*accept_encoding;
	night_table_elt_t                  			*accept_language;
	night_table_elt_t                  			*content_length;
	night_table_elt_t                  			*transfer_encoding;
	night_table_elt_t                  			*keep_alive;
	night_table_elt_t                  			*range;
	
	off_t                             			content_length_n;
	time_t										keep_alive_n;
	
	unsigned                          			connection_type:8;
	unsigned                          			msie:1;
	unsigned                          			msie6:1;
	unsigned                          			opera:1;
	unsigned                          			gecko:1;
    unsigned                          			chrome:1;
    unsigned                          			safari:1;
    unsigned                          			konqueror:1;
    unsigned                          			chunked:1;
};

struct night_http_headers_out_s
{
	int                        					status;
	
	night_list_t								headers;
	
	night_table_elt_t                  			*etag;
	night_table_elt_t                  			*last_modified;
	night_table_elt_t                  			*content_length;
	night_table_elt_t                  			*server;
	night_table_elt_t                  			*date;
	night_table_elt_t                  			*accept_ranges;
	
	night_str_t                         		content_type;
	size_t                            			content_type_len;
	
	off_t                             			content_length_n;
	time_t                            			last_modified_time;
	night_str_t                         		charset;
	
	night_str_t                         		status_line;
};

struct night_http_request_s
{
	night_pool_t 						*pool;
	night_http_request_t               	*main;
	
	void                            	**ctx;
    void                            	**main_conf;
    void                            	**srv_conf;
    void                            	**loc_conf;
    
	night_http_event_handler_pt         read_event_handler;
    night_http_event_handler_pt         write_event_handler;
    
	int									count;
	time_t 								start_sec;
	time_t 								start_msec;
	night_http_connection_t				*http_connection;
	night_connection_t					*connection;
	
	int                         		phase_handler;
	
	int									state;
	uint32_t							method;
    uint32_t							http_version;

    night_buf_t                        	*header_in;
    night_str_t                         request_line;
    
    char								*request_start;
    char   		                        *request_end;
    off_t								request_length;
    char								*method_end;
    char                           		*schema_start;
    char								*schema_end;
	char                           		*host_start;
	char                           		*host_end;
	char                           		*port_start;
 	char                           		*port_end;
	char								*uri_start;
	char   		                        *uri_end;
	char                           		*uri_ext;
	char								*args_start;
	char                           		*header_name_start;
    char	                           	*header_name_end;
    char	                           	*header_start;
    char	                           	*header_end;
	
    night_str_t							method_name;
    night_str_t							uri;
    // 未处理的原始 URI
    night_str_t							unparsed_uri;
    night_str_t                         http_protocol;
    night_str_t                         schema;
    night_str_t							exten;
    night_str_t							args;
    
	uint64_t	                        header_hash;
    uint64_t	                        lowcase_index;
    char                            	lowcase_header[NIGHT_HTTP_LC_HEADER_LEN];
    
    night_http_headers_in_t				headers_in;
	night_http_headers_out_t            headers_out;
	
	night_http_handler_pt               content_handler;
	
	int									err_status;
	
	size_t                            	header_size;
	
	night_chain_t                      	*out;
	
	night_http_posted_request_t        	*posted_requests;
	
	unsigned                          	http_minor:16;
    unsigned                          	http_major:16;
    
	unsigned                          	blocked:8;
    unsigned                          	http_state:8;
    
	unsigned                          	uri_changed:1;
	
	// r->uri_changes 是一个计数器，初始值通常为 NGX_HTTP_MAX_URI_CHANGES（默认为 10）。
	//每次内部重定向时减 1，防止无限重定向循环（如 rewrite 规则错误导致死循环）。
	//如果减到 0，说明重定向次数超限。
	unsigned                          	uri_changes:4;
	
	// URI with "/." and on Win32 with "//" 
    unsigned                          	complex_uri:1;

    // URI with "%" 
    unsigned                          	quoted_uri:1;

    // URI with "+" */
    unsigned                          	plus_in_uri:1;
    
	// URI with empty path 
    unsigned                          	empty_path_in_uri:1;
    //	如果是 empty_path_in_uri（URI 为空但被补成 /），则原始 URI 实际上是无效的（因为客户端发的是空 URI），所以设为 0。
 	// 否则设为 1，表示 unparsed_uri 是有效的原始请求内容。
    unsigned							valid_unparsed_uri:1;
    // 无效的 request header
    unsigned                          	invalid_header:1;
    // r->internal 为 1 表示这是一个内部重定向、子请求或 error_page 跳转等内部生成的请求
    unsigned                          	internal:1;
    // 是否启用 keepalive
    unsigned                          	keepalive:1;
    // lingering_close 表示即使响应已发送完毕， 仍会尝试读取并丢弃客户端可能继续发送的请求体数据, 避免 TCP RST 导致客户端异常
    unsigned                          	lingering_close:1;
    
    unsigned                          	valid_location:1;
    
    unsigned                          	limit_conn_status:2;
    unsigned                          	limit_req_status:3;
    
    unsigned                          	header_sent:1;
    
    unsigned                          	header_only:1;
    
    unsigned                          	chunked:1;
    
    unsigned                          	request_output:1;
    
    unsigned                          	limit_rate_set:1;
};

extern night_http_header_t night_http_headers_in[];

void
night_http_empty_handler(night_event_t *wev);

void
night_http_wait_request_handler(night_event_t *rev);

night_http_request_t*
night_http_create_request(night_connection_t *c);

night_http_request_t*
night_http_alloc_request(night_connection_t *c);

void
night_http_process_request_line(night_event_t *rev);

void
night_http_close_request(night_http_request_t *r, int rc);

void
night_http_free_request(night_http_request_t *r, int rc);

ssize_t
night_http_read_request_header(night_http_request_t *r);

int
night_http_process_request_uri(night_http_request_t *r);

int
night_http_validate_host(night_str_t *host, night_pool_t *pool, int alloc);

void
night_http_finalize_request(night_http_request_t *r, int rc);

int
night_http_set_virtual_server(night_http_request_t *r, night_str_t *host);

void
night_http_process_request_headers(night_event_t *rev);

int
night_http_alloc_large_header_buffer(night_http_request_t *r, int request_line);

void
night_http_run_posted_requests(night_connection_t *c);

int
night_http_process_host(night_http_request_t *r, night_table_elt_t *h, size_t offset);

int
night_http_process_request_header(night_http_request_t *r);

void
night_http_process_request(night_http_request_t *r);

int
night_http_process_connection(night_http_request_t *r, night_table_elt_t *h, size_t offset);

int
night_http_process_header_line(night_http_request_t *r, night_table_elt_t *h, size_t offset);

int
night_http_process_user_agent(night_http_request_t *r, night_table_elt_t *h, size_t offset);

void
night_http_block_reading(night_http_request_t *r);

void
night_http_request_handler(night_event_t *ev);

void
night_http_request_empty_handler(night_http_request_t *r);

void
night_http_finalize_connection(night_http_request_t *r);

void
night_http_close_request(night_http_request_t *r, int rc);

void
night_http_set_keepalive(night_http_request_t *r);

void
night_http_keepalive_handler(night_event_t *rev);

#endif /* _NIGHT_HTTP_REQUEST_H_ */

