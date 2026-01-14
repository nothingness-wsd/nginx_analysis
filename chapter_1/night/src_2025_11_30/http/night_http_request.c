#include "night_core.h"
#include "night_http_request.h"
#include "night_http_connection.h"
#include "night_http_core_module.h"
#include "night_event.h"
#include "night_connection.h"
#include "night_buf.h"
#include "night_pool.h"
#include "night_event_timer.h"
#include "night_time.h"
#include "night_http_parse.h"
#include "night_hash.h"
#include "night_http_module.h"
#include "night_string.h"
#include "night_module.h"
#include "night_cycle.h"
#include "night_event_timer.h"
#include "night_event_posted.h"


// night_http_headers_in 是一个静态数组（通常在 src/http/ngx_http_request.c 中定义），包含所有 Nginx 支持的标准 HTTP 请求头（如 Host、Connection、Content-Length 等）
// 每个元素是 ngx_http_header_t 结构。
// 循环终止条件是 header->name.len == 0，即遇到一个空名称的条目（作为数组结束标志）。
night_http_header_t night_http_headers_in[] = 
{
    { night_string("Host"), 			offsetof(night_http_headers_in_t, host), 			night_http_process_host },
	{ night_string("Connection"), 		offsetof(night_http_headers_in_t, connection), 		night_http_process_connection },
	{ night_string("User-Agent"), 		offsetof(night_http_headers_in_t, user_agent), 		night_http_process_user_agent },
	{ night_string("Accept"), 			offsetof(night_http_headers_in_t, accept), 			night_http_process_header_line },     
	{ night_string("Accept-Encoding"),	offsetof(night_http_headers_in_t, accept_encoding),	night_http_process_header_line },
	{ night_string("Accept-Language"),	offsetof(night_http_headers_in_t, accept_language),	night_http_process_header_line }, 
	
    { night_null_string, 0, NULL }
};

void
night_http_empty_handler(night_event_t *wev)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"function:\t" "night_http_empty_handler\n\n" );

    return;
}

/*
用于处理新建立连接上等待客户端 HTTP 请求的事件
*/
void
night_http_wait_request_handler(night_event_t *rev)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"function:\t" "night_http_wait_request_handler\n\n" );
 
	night_connection_t 			*c;
	night_http_connection_t 	*hc;
	night_http_core_srv_conf_t 	*scf;
	night_buf_t 				*b;
	ssize_t 					n;
	size_t                     	size;
     
    // 从事件结构中获取关联的连接对象 
	c = rev->data;
    
	if (rev->timedout) 
	{   
		dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
		dprintf(trace_file_fd,"读事件超时\n\n");
		
        night_http_close_connection(c);
        return;
    }
    
	if (c->close) 
	{   
		dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    	dprintf(trace_file_fd,"需要关闭连接\n\n");
    	
        night_http_close_connection(c);
        return;
    }
    
    // 从连接对象中获取 HTTP 连接结构
    hc = c->data;
    
    scf = hc->server;
    
    // 设置客户端请求头缓冲区大小
    size = scf->client_header_buffer_size;
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd, "client_header_buffer_size=%ld\n\n", size );
    
	// 获取连接的缓冲区指针
    b = c->buffer;
    
    // 如果连接没有缓冲区，创建一个新的临时缓冲区
	if (b == NULL) 
	{   
        b = night_create_temp_buf(c->pool, size);
        if (b == NULL) 
        {
            night_http_close_connection(c);
            return;
        }

        c->buffer = b;
    }
    // 如果缓冲区存在但未分配内存，分配内存并初始化缓冲区指针
	else if (b->start == NULL) 
	{
        b->start = night_pmalloc(c->pool, size);
        if (b->start == NULL) 
        {
            night_http_close_connection(c);
            return;
        }

        b->pos = b->start;
        b->last = b->start;
        b->end = b->last + size;
    }
    
	// 从客户端连接读取数据到缓冲区
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
	dprintf(trace_file_fd, "从客户端连接读取数据到缓冲区\nn = c->recv(c, b->last, size)\n\n");
	
    n = c->recv(c, b->last, size);
    
    if (n > 0)
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    	dprintf(trace_file_fd, "n = c->recv(c, b->last, size) completed\nn=%ld\n%s\n\n", n, b->start);
    }	
    
    // 当前没有数据可读，需要等待下一次可读事件
	if (n == NIGHT_AGAIN) 
	{
        if (!rev->timer_set) 
        {
        	// 如果定时器未设置，则添加一个 client_header_timeout（默认 60s）的定时器
            night_event_add_timer(rev, scf->client_header_timeout);
            
            // 标记该连接为“可复用”（即 idle keep-alive 连接），允许放入 free_connection 队列以节省资源。
            night_reusable_connection(c, 1);
        }

        if (night_handle_read_event(rev) != NIGHT_OK) 
        {
            night_http_close_connection(c);
            return;
        }

        // We are trying to not hold c->buffer's memory for an idle connection. 
        if (night_plfree(c->pool, b->start) == NIGHT_OK) 
        {
            b->start = NULL;
        }

        return;
    }
    
	if (n == NIGHT_ERROR) 
	{
		dprintf(error_log_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
		dprintf(error_log_fd, "recv data failed\n\n");
		
        night_http_close_connection(c);
        return;
    }
    
	if (n == 0) 
	{
		dprintf(error_log_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
		dprintf(error_log_fd, "client closed connection\n\n");
		
        night_http_close_connection(c);
        return;
    }
    
    // 更新缓冲区数据末尾指针
    b->last += n;
    
    // 标记连接为不可重用
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
	dprintf(trace_file_fd, "标记连接为不可重用\nngx_reusable_connection(c, 0)\n\n");
	
    night_reusable_connection(c, 0);
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd, "ngx_reusable_connection(c, 0) completed\n\n");   
    
    // 为连接创建 HTTP 请求结构
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
	dprintf(trace_file_fd, "为连接创建 HTTP 请求结构\nc->data = night_http_create_request(c)\n\n");
	 
	c->data = night_http_create_request(c);
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd, "night_http_create_request(c) completed\n\n");
    
    if (c->data == NULL) 
    {
        night_http_close_connection(c);
        return;
    }
    
	//更新读事件的处理函数
	//状态转换，从"等待请求"状态转换到"处理请求行"状态
    rev->handler = night_http_process_request_line;
    
    // 立即调用请求行处理函数
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd, "调用请求行处理函数\nnight_http_process_request_line(rev)\n\n");
    
    night_http_process_request_line(rev);
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd, "night_http_process_request_line completed\n\n");
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd, "function night_http_wait_request_handler:\treturn\n\n");
    
}

night_http_request_t*
night_http_create_request(night_connection_t *c)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"function:\t" "night_http_create_request\n\n");
    
    night_http_request_t *r;
    
	r = night_http_alloc_request(c);
    if (r == NULL) 
    {
        return NULL;
    }
    
    c->requests++;
    
    return r;
}

night_http_request_t*
night_http_alloc_request(night_connection_t *c)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"function:  night_http_alloc_request\n\n");
    
    night_http_request_t 	*r;
    night_pool_t 			*pool;
    night_time_t			*tp;
    night_http_connection_t	*hc;
    night_http_conf_t		*http_conf;
    
    hc = c->data;
    
	pool = night_create_pool(night_page_size);
    if (pool == NULL) 
    {
        return NULL;
    }
    
	r = night_pmalloc(pool, sizeof(night_http_request_t));
    if (r == NULL) 
    {
        night_destroy_pool(pool);
        return NULL;
    }
    
    r->pool = pool;
    
    // 为当前 HTTP 请求（r）分配一个指针数组，用于存储每个 HTTP 模块在该请求上的私有上下文数据
	r->ctx = night_pmalloc(r->pool, sizeof(void *) * night_http_modules_n);
    if (r->ctx == NULL) 
    {
        night_destroy_pool(r->pool);
        
        return NULL;
    }
    
    r->http_connection = hc;
    r->connection = c;
    
    http_conf = hc->server->ctx;
    
	r->main_conf = http_conf->main_conf;
    r->srv_conf = http_conf->srv_conf;
    r->loc_conf = http_conf->loc_conf;
    
    r->read_event_handler = night_http_block_reading;
    
	if (night_list_init(&r->headers_out.headers, r->pool, 20, sizeof(night_table_elt_t)) != NIGHT_OK)
    {
        night_destroy_pool(r->pool);
        return NULL;
    }
    
    r->header_in = c->buffer;
    
	r->main = r;
	r->count = 1;
	
	tp = night_cached_time;
    r->start_sec = tp->sec;
    r->start_msec = tp->msec;
    
	r->method = NIGHT_HTTP_UNKNOWN;
    r->http_version = NIGHT_HTTP_VERSION_10;
    
    r->uri_changes = NIGHT_HTTP_MAX_URI_CHANGES + 1;
    
	r->headers_in.content_length_n = -1;
    r->headers_in.keep_alive_n = -1;
    r->headers_out.content_length_n = -1;
    r->headers_out.last_modified_time = -1;
    
    return r;
}    


void
night_http_process_request_line(night_event_t *rev)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"function:\t" "night_http_process_request_line\n\n");
    
	night_connection_t    	*c;
    night_http_request_t  	*r;
    int						rc,rv;
    ssize_t					n;
    night_str_t            	host;
    
    c = rev->data;
    r = c->data;
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd, "rev->timedout=%d\n\n", rev->timedout);
    
	if (rev->timedout) 
	{
        c->timedout = 1;
        
        night_http_close_request(r, NIGHT_HTTP_REQUEST_TIME_OUT);
        return;
    }
    
    // 表示“需要更多数据才能继续解析”
    rc = NIGHT_AGAIN;
	
	// 开启一个 无限循环，用于持续读取和解析请求行，直到成功、出错或需要等待更多数据
	for ( ; ; )
	{
		// 如果上一次解析结果是 NIGHT_AGAIN（即数据不足），则调用 night_http_read_request_header(r) 从客户端读取更多请求头数据（包括请求行）。
		// night_http_read_request_header() 会尝试从 socket 读取数据到 r->header_in 缓冲区。
		// 如果读取返回 NIGHT_AGAIN（无数据可读，但连接未断）或 NIGHT_ERROR（读取出错），则跳出循环，等待下次事件触发。
		// 此时不会关闭连接，而是等待 epoll 等再次通知可读。
		if (rc == NIGHT_AGAIN) 
		{
            n = night_http_read_request_header(r);
            
            dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
            dprintf(trace_file_fd, "%ld = night_http_read_request_header(r)\n\n", n);

            if (n == NIGHT_AGAIN || n == NIGHT_ERROR) 
            {
                break;
            }
        }
        
        rc = night_http_parse_request_line(r, r->header_in);
        
        // 解析成功
		if (rc == NIGHT_OK) 
		{
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
			dprintf(trace_file_fd, "if (rc == NIGHT_OK) \n\n");
			
			// The request line has been parsed successfully
            r->request_line.len = r->request_end - r->request_start;
            r->request_line.data = r->request_start;
            r->request_length = r->header_in->pos - r->request_start;
            
            dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
            dprintf(trace_file_fd, "request_line=\n");
			write(trace_file_fd, r->request_line.data, r->request_line.len);
			dprintf(trace_file_fd, "\n\n");
            
			r->method_name.len = r->method_end - r->request_start;
            r->method_name.data = r->request_line.data;
            
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
            dprintf(trace_file_fd, "method_name=\n");
            write(trace_file_fd, r->method_name.data, r->method_name.len);
            dprintf(trace_file_fd, "\n\n");
            
            // 设置 HTTP 协议版本字符串
			if (r->http_protocol.data) 
			{
                r->http_protocol.len = r->request_end - r->http_protocol.data;
                
                dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
                dprintf(trace_file_fd, "r->http_protocol=\n");
                write(trace_file_fd, r->http_protocol.data, r->http_protocol.len);
                dprintf(trace_file_fd, "\n\n");
            }

			// 处理 URI（可能包含 schema 和 host）
			if (night_http_process_request_uri(r) != NIGHT_OK) 
			{
				dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
				dprintf(trace_file_fd, "if (night_http_process_request_uri(r) != NIGHT_OK)\n\n");
				
                break;
            }
            
			// 如果 URI 中包含 schema（如 http://）
			if (r->schema_end) 
			{
				dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
				dprintf(trace_file_fd, "if (r->schema_end)\n\n");
				
                r->schema.len = r->schema_end - r->schema_start;
                r->schema.data = r->schema_start;
            }
            
            // 如果 URI 中包含 host
			if (r->host_end)
			{
				dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
				dprintf(trace_file_fd, "if (r->host_end)\n\n");
				
                host.len = r->host_end - r->host_start;
                host.data = r->host_start;

				// 验证 host 合法性
                rc = night_http_validate_host(&host, r->pool, 0);
				
				// 如果 host 无效（NGX_DECLINED），记录日志，并返回 400 Bad Request。
                if (rc == NIGHT_DECLINED) 
                {
                	dprintf(error_log_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
                	dprintf(error_log_fd, "client sent invalid host in request line\n\n");
                	
                    night_http_finalize_request(r, NIGHT_HTTP_BAD_REQUEST);
                    
                    break;
                }
				
				// 如果验证过程中发生内部错误（如内存分配失败），返回 500 Internal Server Error。
                if (rc == NIGHT_ERROR) 
                {
                    night_http_close_request(r, NIGHT_HTTP_INTERNAL_SERVER_ERROR);
                    break;
                }
				
				// 根据 host 设置虚拟服务器（virtual server）
				// 根据 host 查找对应的 server{} 块（虚拟主机）。
				// 如果找不到或出错，跳出循环（后续会关闭请求）。
                if (night_http_set_virtual_server(r, &host) == NIGHT_ERROR) 
                {
                    break;
                }
				// 将解析出的 host 保存到 r->headers_in.server，供后续模块使用
                r->headers_in.server = host;
            }
            
            // 初始化请求头列表
            // 为请求头（Headers）初始化一个链表，预分配 20 个 ngx_table_elt_t（键值对结构）。
			// 如果失败（内存不足），返回 500 并退出。
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
			dprintf(trace_file_fd, "night_list_init r->headers_in.headers\n\n");
			
			if (night_list_init(&r->headers_in.headers, r->pool, 20, sizeof(night_table_elt_t)) != NIGHT_OK)
            {	
				dprintf(error_log_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
            	dprintf(error_log_fd, "night_list_init failed\n\n");
            	
				dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
            	dprintf(trace_file_fd, "night_http_close_request\n\n");
            	
                night_http_close_request(r, NIGHT_HTTP_INTERNAL_SERVER_ERROR);
                break;
            }
            
            // 切换事件处理器为处理请求头
			rev->handler = night_http_process_request_headers;
			
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
			dprintf(trace_file_fd, "rev->handler = night_http_process_request_headers;\n\n");
			
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
			dprintf(trace_file_fd, "night_http_process_request_headers(rev)\n\n");
			
            night_http_process_request_headers(rev);
            
            dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
            dprintf(trace_file_fd, "night_http_process_request_headers(rev) completed\n\n");

            break;  
		}
         
        // 解析出错 
		if (rc != NIGHT_AGAIN) 
		{
			// there was error while a request line parsing
			dprintf(error_log_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
			dprintf(error_log_fd, "Parse request line failed\n\n");

			// 如果是版本不支持（如 HTTP/3.0），返回 505 HTTP Version Not Supported。
			// 其他错误一律返回 400 Bad Request。
            if (rc == NIGHT_HTTP_PARSE_INVALID_VERSION) 
            {
                night_http_finalize_request(r, NIGHT_HTTP_VERSION_NOT_SUPPORTED);
            } 
            else 
            {
                night_http_finalize_request(r, NIGHT_HTTP_BAD_REQUEST);
            }

            break;
		}
		
		// NIGHT_AGAIN: a request line parsing is still incomplete
		// 数据不足（rc == NIGHT_AGAIN），且缓冲区已满
        if (r->header_in->pos == r->header_in->end) 
        {
        	// 如果解析返回 NGX_AGAIN，且当前缓冲区已满（pos == end），说明需要更大的缓冲区。
        	// 调用 night_http_alloc_large_header_buffer(r, 1) 分配更大的缓冲区（第二个参数 1 表示这是为请求行分配，而非请求头）
            rv = night_http_alloc_large_header_buffer(r, 1);

			// 如果分配失败（内存不足），返回 500
            if (rv == NIGHT_ERROR) 
            {
                night_http_close_request(r, NIGHT_HTTP_INTERNAL_SERVER_ERROR);
                break;
            }

			// 如果返回 NIGHT_DECLINED，表示 URI 长度超过 large_client_header_buffers 限制。
			// 构造当前已读取的请求行（用于日志），记录“URI too long”，返回 414 Request-URI Too Large。
            if (rv == NIGHT_DECLINED) 
            {
                r->request_line.len = r->header_in->end - r->request_start;
                r->request_line.data = r->request_start;

				dprintf(error_log_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
				dprintf(error_log_fd, "client sent too long URI\n\n");
				
                night_http_finalize_request(r, NIGHT_HTTP_REQUEST_URI_TOO_LARGE);
                break;
            }
        }
		
	}
    // 执行该连接上延迟处理的请求（posted requests）
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
	dprintf(trace_file_fd, "night_http_run_posted_requests(c)\n\n");
	
    night_http_run_posted_requests(c);
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd, "night_http_run_posted_requests(c) completed\n\n");
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd, "function night_http_process_request_line:\treturn\n\n");
}

void
night_http_close_request(night_http_request_t *r, int rc)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"function:\t" "night_http_close_request\n\n");
    
    night_connection_t  *c;
    
    r = r->main;
    c = r->connection;

    if (r->count == 0) 
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    	dprintf(trace_file_fd, "http request count is zero\n\n" );
    }

    r->count--;

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
	dprintf(trace_file_fd, "r->count=%d\n\n", r->count);
	
	// r->count：仍有其他模块/事件持有该请求，不能释放。
	// r->blocked：表示请求被“阻塞”，通常是因为某些异步操作尚未完成，需等待
    if (r->count || r->blocked) 
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    	dprintf(trace_file_fd, "if (r->count || r->blocked)\nreturn;\n\n");
    	
        return;
    }

    night_http_free_request(r, rc);
    night_http_close_connection(c);
}

/*
该函数用于释放和清理 HTTP 请求对象
*/
void
night_http_free_request(night_http_request_t *r, int rc)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"function:\t" "night_http_free_request\n\n");
    
    night_pool_t		*pool;
    
    // 如果 r->pool 已为 NULL，说明请求已被释放过（重复调用）
	if (r->pool == NULL) 
	{
		dprintf(error_log_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
		dprintf(error_log_fd, "http request already closed\n\n");
		
        return;
    }
    
    // rc > 0：表示传入的是有效 HTTP 状态码
    // 且尚未设置 headers_out.status（为 0），或尚未发送任何数据（sent == 0）。
	// 此时将 rc 赋给 r->headers_out.status，作为最终响应状态。
	if (rc > 0 && (r->headers_out.status == 0 || r->connection->sent == 0)) 
	{
        r->headers_out.status = rc;
    }
    
    // the various request strings were allocated from r->pool 
    r->request_line.len = 0;

    r->connection->destroyed = 1;
    
	pool = r->pool;
    r->pool = NULL;

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
	dprintf(trace_file_fd, "night_destroy_pool(pool);\n\n");
	
    night_destroy_pool(pool);
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd, "function night_http_free_request:\treturn;\n\n");
    
    return;
}

ssize_t
night_http_read_request_header(night_http_request_t *r)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"function:\t" "night_http_read_request_header\n\n");
    
    ssize_t						n;
    night_event_t               *rev;
    night_connection_t          *c;
    night_http_core_srv_conf_t	*cscf;
    
    c = r->connection;
    rev = c->read;
    
    // 计算当前请求头缓冲区中尚未处理的数据长度
    // pos 指向已解析的位置，last 指向已接收数据的末尾
    n = r->header_in->last - r->header_in->pos;
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd, "n=%ld\n\n", n);
    
    // 如果缓冲区中还有未消费的数据，直接返回其长度。
	if (n > 0) 
	{
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
		dprintf(trace_file_fd, "if (n > 0)\nreturn n;\n\n");
		
        return n;
    }
    
    // rev->ready：表示该连接的读事件是否就绪（即 socket 是否有数据可读）
	if (rev->ready) 
	{
		// 尝试从 socket 读取更多数据
        n = c->recv(c, r->header_in->last, r->header_in->end - r->header_in->last);
    } 
    else 
    {
        n = NIGHT_AGAIN;
    }
    
	if (n == NIGHT_AGAIN) 
	{
		// 如果读事件尚未设置超时定时器，则为其添加一个
        if (!rev->timer_set) 
        {
            cscf = r->http_connection->server;
            
            dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
            dprintf(trace_file_fd,"client_header_timeout=%ld\n\n", cscf->client_header_timeout);
            
            night_event_add_timer(rev, cscf->client_header_timeout);
        }

		// 重新注册读事件到事件循环中
        if (night_handle_read_event(rev) != NIGHT_OK) 
        {
            night_http_close_request(r, NIGHT_HTTP_INTERNAL_SERVER_ERROR);
            
            return NIGHT_ERROR;
        }

        return NIGHT_AGAIN;
    }
    
    return n;
}

int
night_http_process_request_uri(night_http_request_t *r)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"function:\t" "night_http_process_request_uri\n\n");
    
    // 如果存在查询参数，则 URI 长度 = args_start - 1 - uri_start（减 1 是为了排除 ? 本身）。
	// 否则，URI 长度 = uri_end - uri_start（整个 URI 都是路径部分）。
	if (r->args_start) 
	{
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
		dprintf(trace_file_fd, "if (r->args_start) \n");
		dprintf(trace_file_fd, "r->uri.len = r->args_start - 1 - r->uri_start\n");
		
        r->uri.len = r->args_start - 1 - r->uri_start;
        
        dprintf(trace_file_fd, "r->uri.len=%ld\n\n", r->uri.len);
    } 
    else 
    {
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
		dprintf(trace_file_fd, "else\n");
		dprintf(trace_file_fd, "r->uri.len = r->uri_end - r->uri_start\n");
		
        r->uri.len = r->uri_end - r->uri_start;
        
        dprintf(trace_file_fd, "r->uri.len=%ld\n\n", r->uri.len);
    }
    
    // 复杂处理
    if (r->complex_uri || r->quoted_uri || r->empty_path_in_uri) 
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    	dprintf(trace_file_fd, "if (r->complex_uri || r->quoted_uri || r->empty_path_in_uri) \n\n");
    }
    else 
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    	dprintf(trace_file_fd, "r->uri.data = r->uri_start\n");
    	dprintf(trace_file_fd, "r->uri.data=");
    	
        r->uri.data = r->uri_start;
        
        write(trace_file_fd, r->uri.data, r->uri.len);
        dprintf(trace_file_fd, "\n\n");
    }
    
    // 设置 unparsed_uri：保存 原始未处理的 URI 字符串（包括查询参数）。
	r->unparsed_uri.len = r->uri_end - r->uri_start;
    r->unparsed_uri.data = r->uri_start;
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd, "r->unparsed_uri=");
    write(trace_file_fd, r->unparsed_uri.data, r->unparsed_uri.len);
    dprintf(trace_file_fd, "\n\n");
    
    // 设置 valid_unparsed_uri 标志：
	// 如果是 empty_path_in_uri（URI 为空但被补成 /），则原始 URI 实际上是无效的（因为客户端发的是空 URI），所以设为 0。
 	// 否则设为 1，表示 unparsed_uri 是有效的原始请求内容。
	r->valid_unparsed_uri = r->empty_path_in_uri ? 0 : 1;
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
	dprintf(trace_file_fd, "r->valid_unparsed_uri=%d\n\n", r->valid_unparsed_uri);
    
    // r->uri_ext 是在之前解析请求行时记录的 文件扩展名起始位置
	// 如果存在扩展名：
	// 计算扩展名长度（是否包含查询参数）。
	if (r->uri_ext) 
	{
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
		dprintf(trace_file_fd, "if (r->uri_ext)\n\n");
		
        if (r->args_start) 
        {
            r->exten.len = r->args_start - 1 - r->uri_ext;
        } else 
        {
            r->exten.len = r->uri_end - r->uri_ext;
        }

        r->exten.data = r->uri_ext;
    }

	// 如果存在查询参数（? 后面的部分）：
	// 设置 r->args 为查询字符串（如 ?name=value 中的 name=value）。
    if (r->args_start && r->uri_end > r->args_start) 
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    	dprintf(trace_file_fd, "if (r->args_start && r->uri_end > r->args_start)\n\n");
    	
        r->args.len = r->uri_end - r->args_start;
        r->args.data = r->args_start;
    }
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd, "return NIGHT_OK;\n\n");
    
    return NIGHT_OK;
}

int
night_http_validate_host(night_str_t *host, night_pool_t *pool, int alloc)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"function:\t" "night_http_validate_host\n\n");
    
    int rc;
    
    return rc;
}

/*
ngx_http_finalize_request 是 Nginx HTTP 模块中用于终结 HTTP 请求的关键函数。它处理各种请求完成情况，包括正常完成、错误处理、子请求处理等，并负责清理资源、设置连接状态等。
*/
void
night_http_finalize_request(night_http_request_t *r, int rc)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"function:\t" "night_http_finalize_request\n\n");
    
    night_connection_t			*c;
    
    c = r->connection;
    
	// 如果返回码是 NGX_DONE（表示请求已完成但连接需要保持），调用连接终结函数并返回
    if (rc == NIGHT_DONE) 
    {	
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    	dprintf(trace_file_fd, "if (rc == NIGHT_DONE) {\nnight_http_finalize_connection(r);\n\n");
    	
        night_http_finalize_connection(r);
        
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        dprintf(trace_file_fd, "night_http_finalize_connection(r) completed\n\n");
        
        dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        dprintf(trace_file_fd, "night_http_finalize_request\treturn;\n\n");
        
        return;
    }
    
    //最终调用连接终结函数，完成整个请求处理流程
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
	dprintf(trace_file_fd, "night_http_finalize_connection(r);\n\n");
	
    night_http_finalize_connection(r);
    
    return;
}

/*
负责处理HTTP请求的最终清理工作，决定连接是保持活跃状态还是关闭连接，以及是否需要延迟关闭来处理剩余数据
*/
void
night_http_finalize_connection(night_http_request_t *r)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
	dprintf(trace_file_fd, "function:\t" "night_http_finalize_connection\n\n");
	
	night_http_core_loc_conf_t  			*clcf;
	
	clcf = r->loc_conf[night_http_core_module.ctx_index];
	
	// 子请求
	if (r->main->count != 1) 
	{
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
		dprintf(trace_file_fd, "if (r->main->count != 1)\n\n");
		
		// 关闭当前请求并返回
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
		dprintf(trace_file_fd, "night_http_close_request(r, 0);\n\n");
		
        night_http_close_request(r, 0);
        return;
	}
	
/*
检查多个条件：
服务没有被终止
服务没有在退出
当前请求允许keepalive
keepalive超时时间大于0
如果都满足，设置keepalive状态并返回
*/
    if (!night_terminate
         && !night_exiting
         && r->keepalive)
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    	dprintf(trace_file_fd, "night_http_set_keepalive(r);\n\n");
    	
        night_http_set_keepalive(r);
        
        dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        dprintf(trace_file_fd, "night_http_set_keepalive(r) completed\n\n");
        
        dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        dprintf(trace_file_fd, "function night_http_finalize_connection:\treturn;\n\n");
        
        return;
    }
	
}	

/*
这个函数用于将HTTP请求设置为keepalive状态，即保持连接不关闭，等待处理下一个请求。这是HTTP/1.1持久连接机制的核心实现。
*/
void
night_http_set_keepalive(night_http_request_t *r)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
	dprintf(trace_file_fd, "function:\t" "night_http_set_keepalive\n\n");
	
	night_event_t				*rev, *wev;
    night_connection_t          *c;
    night_http_core_loc_conf_t  *clcf;
    night_http_connection_t     *hc;
    night_buf_t                 *b;
    int							rc;
    
	c = r->connection;
    rev = c->read;
    
    clcf = r->loc_conf[night_http_core_module.ctx_index];
    
	hc = r->http_connection;
    //b = r->header_in;
    
	// 重置请求的keepalive标志（防止递归调用）
	r->keepalive = 0;
	
	// 释放当前HTTP请求对象
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
	dprintf(trace_file_fd, "ngx_http_free_request(r, 0);\n\n");
	
    night_http_free_request(r, 0);
    
	// 将连接的数据指针指向HTTP连接对象
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
	dprintf(trace_file_fd, "c->data = hc;\n\n");
	
    c->data = hc;
    
    // 处理读事件，如果失败则关闭连接
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
	dprintf(trace_file_fd, "night_handle_read_event(rev)\n\n");
	
    rc = night_handle_read_event(rev);
	if (rc != NIGHT_OK) 
	{
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    	dprintf(trace_file_fd, "if (rc != NIGHT_OK)\nnight_http_close_connection(c);\n\n");
    	
        night_http_close_connection(c);
        
        dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        dprintf(trace_file_fd, "function night_http_set_keepalive:\treturn;\n\n");
        
        return;
    }
    
    // 获取写事件对象，设置为空处理器
	wev = c->write;
    wev->handler = night_http_empty_handler;
    
    
    b = c->buffer;
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
	dprintf(trace_file_fd, "b=%p\n", b);
	dprintf(trace_file_fd, "b->start=%p\n\n", b->start);
    
	// 尝试释放连接缓冲区的内存
	//如果释放成功，设置pos为NULL作为标记
	//如果失败，重置缓冲区指针
	rc = night_plfree(c->pool, b->start);
    if (rc == NIGHT_OK) 
    {
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
		dprintf(trace_file_fd, "if (rc == NIGHT_OK)\n");
		dprintf(trace_file_fd, "b->pos = NULL;\n\n");
		
        b->pos = NULL;
    }
	
	// 设置读事件处理器为keepalive处理器
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
	dprintf(trace_file_fd, "rev->handler = night_http_keepalive_handler;\n\n");
	
    rev->handler = night_http_keepalive_handler;
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd, "night_tcp_nodelay\n\n");
    
    rc = night_tcp_nodelay(c);
    if (rc != NIGHT_OK)
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    	dprintf(trace_file_fd, "night_http_close_connection(c);\n\n");
    	
		night_http_close_connection(c);
		
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
		dprintf(trace_file_fd, "function night_http_set_keepalive:\t" "return;\n\n");
		
        return;
    }
    
	// 标记连接为空闲状态
    c->idle = 1;
    
	//将连接设置为可重用
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
	dprintf(trace_file_fd, "night_reusable_connection(c, 1);\n\n");
	
    night_reusable_connection(c, 1);
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
	dprintf(trace_file_fd, "night_reusable_connection(c, 1)\tcompleted\n\n");
    
	// 为读事件添加keepalive超时定时器
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
	dprintf(trace_file_fd, "night_event_add_timer(rev, clcf->keepalive_timeout);\nclcf->keepalive_timeout=%ld\n\n", clcf->keepalive_timeout);
	
	night_event_add_timer(rev, clcf->keepalive_timeout);
	
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"night_event_add_timer completed\n\n");
	
	//如果读事件已经就绪，将其添加到待处理事件队列
	if (rev->ready) 
	{
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    	dprintf(trace_file_fd, "读事件已经就绪，将其添加到待处理事件队列\n\n");
    	
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    	dprintf(trace_file_fd, "night_add_post_event(&night_posted_events, rev);\n\n");
    	
    	night_add_post_event(&night_posted_events, rev);
    }
}	

/*
这个函数是 Nginx HTTP 保持连接（keepalive）的事件处理器，用于处理处于 keepalive 状态的连接，检查是否有新的请求到来。
*/
void
night_http_keepalive_handler(night_event_t *rev)
{	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
	dprintf(trace_file_fd, "function:\t" "night_http_keepalive_handler\n\n");
	
	night_connection_t  		*c;
	
	c = rev->data;

}	
	/*
    size_t             size;
    ssize_t            n;
    ngx_buf_t         *b;
    

    

    ngx_log_debug0(NGX_LOG_DEBUG_HTTP, c->log, 0, "http keepalive handler");

    if (rev->timedout || c->close) {
        ngx_http_close_connection(c);
        return;
    }

#if (NGX_HAVE_KQUEUE)

    if (ngx_event_flags & NGX_USE_KQUEUE_EVENT) {
        if (rev->pending_eof) {
            c->log->handler = NULL;
            ngx_log_error(NGX_LOG_INFO, c->log, rev->kq_errno,
                          "kevent() reported that client %V closed "
                          "keepalive connection", &c->addr_text);
#if (NGX_HTTP_SSL)
            if (c->ssl) {
                c->ssl->no_send_shutdown = 1;
            }
#endif
            ngx_http_close_connection(c);
            return;
        }
    }

#endif

    b = c->buffer;
    size = b->end - b->start;

    if (b->pos == NULL) {

        /*
         * The c->buffer's memory was freed by ngx_http_set_keepalive().
         * However, the c->buffer->start and c->buffer->end were not changed
         * to keep the buffer size.
         *

        b->pos = ngx_palloc(c->pool, size);
        if (b->pos == NULL) {
            ngx_http_close_connection(c);
            return;
        }

        b->start = b->pos;
        b->last = b->pos;
        b->end = b->pos + size;
    }

    /*
     * MSIE closes a keepalive connection with RST flag
     * so we ignore ECONNRESET here.
     *

    c->log_error = NGX_ERROR_IGNORE_ECONNRESET;
    ngx_set_socket_errno(0);

    n = c->recv(c, b->last, size);
    c->log_error = NGX_ERROR_INFO;

    if (n == NGX_AGAIN) {
        if (ngx_handle_read_event(rev, 0) != NGX_OK) {
            ngx_http_close_connection(c);
            return;
        }

        /*
         * Like ngx_http_set_keepalive() we are trying to not hold
         * c->buffer's memory for a keepalive connection.
         *

        if (ngx_pfree(c->pool, b->start) == NGX_OK) {

            /*
             * the special note that c->buffer's memory was freed
             *

            b->pos = NULL;
        }

        return;
    }

    if (n == NGX_ERROR) {
        ngx_http_close_connection(c);
        return;
    }

    c->log->handler = NULL;

    if (n == 0) {
        ngx_log_error(NGX_LOG_INFO, c->log, ngx_socket_errno,
                      "client %V closed keepalive connection", &c->addr_text);
        ngx_http_close_connection(c);
        return;
    }

    b->last += n;

    c->log->handler = ngx_http_log_error;
    c->log->action = "reading client request line";

    c->idle = 0;
    ngx_reusable_connection(c, 0);

    c->data = ngx_http_create_request(c);
    if (c->data == NULL) {
        ngx_http_close_connection(c);
        return;
    }

    c->sent = 0;
    c->destroyed = 0;

    ngx_del_timer(rev);

    rev->handler = ngx_http_process_request_line;
    ngx_http_process_request_line(rev);
}
*/

int
night_http_set_virtual_server(night_http_request_t *r, night_str_t *host)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"function:\t" "night_http_set_virtual_server\n\n");
    
    int rc;
    
    return rc;
}

void
night_http_process_request_headers(night_event_t *rev)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"function:\t" "night_http_process_request_headers\n\n");
    
    night_connection_t			*c;
    night_http_request_t		*r;
    int							rc;
    int							rv;
    int							n;
    night_table_elt_t			*h;
    night_http_header_t			*hh;
    night_http_core_main_conf_t	*cmcf;
    
	c = rev->data;
    r = c->data;
    
    // 如果读事件超时（rev->timedout 为真），记录错误日志。
	// 标记连接超时（c->timedout = 1）。
	// 关闭请求，返回 408 Request Timeout。
	if (rev->timedout) 
	{
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
		dprintf(trace_file_fd, "if (rev->timedout)\n\n");
		
		dprintf(error_log_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
		dprintf(error_log_fd, "client timed out\n\n");
		
        c->timedout = 1;
        night_http_close_request(r, NIGHT_HTTP_REQUEST_TIME_OUT);
        return;
    }
    
    cmcf = r->main_conf[night_http_core_module.ctx_index];
    
    rc = NIGHT_AGAIN;
    
    // 主循环：持续解析请求头
    // 初始化：rc = NIGHT_AGAIN 表示尚未开始解析，需要先读取数据。
	// 无限循环：直到遇到错误、完成解析或需要等待更多数据。
    for ( ;; )
    {
    	// 表示上一次解析未完成，需要继续读取。
    	if (rc == NIGHT_AGAIN) 
    	{
    		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    		dprintf(trace_file_fd, "if (rc == NIGHT_AGAIN) \n\n");
    		
    		//检查缓冲区是否已满
    		if (r->header_in->pos == r->header_in->end) 
    		{
    			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    			dprintf(trace_file_fd, "if (r->header_in->pos == r->header_in->end)\n\n");
    			
    			// 尝试分配更大的缓冲区
    			// 参数 0：表示这是用于 header 行（非 body）
    			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    			dprintf(trace_file_fd, "rv = night_http_alloc_large_header_buffer(r, 0)\n\n");
    			rv = night_http_alloc_large_header_buffer(r, 0);

			}
			
			// 读取数据
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
			dprintf(trace_file_fd, "n = night_http_read_request_header(r);\n\n");
			
			n = night_http_read_request_header(r);
			
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
			dprintf(trace_file_fd, "n=%d\n\n", n);
			
			// NIGHT_AGAIN：无数据可读（需等待下一次事件）
			// NIGHT_ERROR：读取失败（如连接关闭）
			if (n == NIGHT_AGAIN || n == NIGHT_ERROR) 
			{
                break;
            }
    	}
    	
    	// 解析 header_in 中的一行 header
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    	dprintf(trace_file_fd, "rc = night_http_parse_header_line(r, r->header_in);\n\n");
    	
    	rc = night_http_parse_header_line(r, r->header_in);
                                        
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
		dprintf(trace_file_fd, "rc=%d\n\n", rc);
		
		if (rc == NIGHT_OK) 
		{
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
			dprintf(trace_file_fd, "if (rc == NIGHT_OK)\n\n");
			
			// 累加已解析的请求长度
			r->request_length += r->header_in->pos - r->header_name_start;
			
			if (r->invalid_header) 
			{
				// 如果该头部语法有误,记录日志，忽略
				dprintf(error_log_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
				dprintf(error_log_fd, "client sent invalid header line: \n");
				write(error_log_fd, r->header_name_start, r->header_end - r->header_name_start);
				dprintf(error_log_fd, "\n\n");
				
				continue;
			}
			
			// a header line has been parsed successfully
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
			dprintf(trace_file_fd, "night_list_push\n\n");
			
            h = night_list_push(&r->headers_in.headers);
            if (h == NULL)
            {
            	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
            	dprintf(trace_file_fd, "if (h == NULL)\n\n");
            	
                night_http_close_request(r, NIGHT_HTTP_INTERNAL_SERVER_ERROR);
                break;
            }
            
            h->hash = r->header_hash;
            
			h->key.len = r->header_name_end - r->header_name_start;
            h->key.data = r->header_name_start;
            h->key.data[h->key.len] = '\0';
            
            dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
            dprintf(trace_file_fd, "h->key=%s\n\n", h->key.data);
            
			h->value.len = r->header_end - r->header_start;
            h->value.data = r->header_start;
            h->value.data[h->value.len] = '\0';
            
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
            dprintf(trace_file_fd, "h->value=%s\n\n", h->value.data);
            
			h->lowcase_key = night_pmalloc(r->pool, h->key.len + 1);
            if (h->lowcase_key == NULL) 
            {
                night_http_close_request(r, NIGHT_HTTP_INTERNAL_SERVER_ERROR);
                break;
            }
            
			if (h->key.len == r->lowcase_index) 
			{
                memcpy(h->lowcase_key, r->lowcase_header, h->key.len);
                
                dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
                dprintf(trace_file_fd, "h->lowcase_key=%s\n\n", h->lowcase_key);
            } 
            else 
            {
                night_strlow(h->lowcase_key, h->key.data, h->key.len);
            }
            
            // 在全局哈希表 headers_in_hash 中查找是否有预定义的处理函数（如 Host、Content-Length 等）。
			hh = night_hash_find(&cmcf->headers_in_hash, h->hash, h->lowcase_key, h->key.len);
			
			if (hh)
			{
				dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
				dprintf(trace_file_fd, "hh->name=%s\n\n", hh->name.data);
			}
			else
			{
				dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
				dprintf(trace_file_fd, "night_hash_find not found\n\n");
			}
			
			// 如果找到处理函数，则调用它
			// 处理函数可能修改请求上下文（如设置 r->headers_in.host），或返回错误（如 Host 无效）
			// 若返回非 NIGHT_OK，则终止 header 解析
            if (hh && hh->handler(r, h, hh->offset) != NIGHT_OK) 
            {
                break;
            }
            // 继续解析下一行 header
            dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
            dprintf(trace_file_fd, "continue;\n\n");
            
            continue; 
		}
		
		// header 解析完成（遇到空行）
		if (rc == NIGHT_HTTP_PARSE_HEADER_DONE) 
		{
			// a whole header has been parsed successfully
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
			dprintf(trace_file_fd, "http header done\n\n");
			
			// 累加最后部分长度
			r->request_length += r->header_in->pos - r->header_name_start;
			
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
			dprintf(trace_file_fd, "r->request_length=%ld\n\n", r->request_length);
			
			// 标记请求状态为“已准备好处理请求体或响应”
			r->http_state = NIGHT_HTTP_PROCESS_REQUEST_STATE;
			
			// 验证请求头合法性
			rc = night_http_process_request_header(r);
			
			// 若验证失败，则不再继续
			if (rc != NIGHT_OK) 
			{
                break;
            }
            
            // 正式进入请求处理阶段
            dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
            dprintf(trace_file_fd, "night_http_process_request(r)\n\n");
            
            night_http_process_request(r);
            
            dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
            dprintf(trace_file_fd, "night_http_process_request(r) completed\n\n");
            
            break;
		}
		
		// 解析器需要更多数据才能完成当前行，继续循环等待读取
		if (rc == NIGHT_AGAIN) 
		{
			// a header line parsing is still not complete
            continue;
		}
		
		// 非法 header, rc == NIGHT_HTTP_PARSE_INVALID_HEADER
		dprintf(error_log_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
		dprintf(error_log_fd, "client sent invalid header line:\n");
		write(error_log_fd, r->header_name_start, r->header_end - r->header_name_start);
		dprintf(error_log_fd, "\n\n");
		
		// 返回 400 Bad Request
        night_http_finalize_request(r, NIGHT_HTTP_BAD_REQUEST);
        break;
    }
    
	// 循环结束后：处理延迟请求
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
	dprintf(trace_file_fd, "night_http_run_posted_requests(c)\n\n");
	
	night_http_run_posted_requests(c);
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
	dprintf(trace_file_fd, "night_http_run_posted_requests(c) completed\n\n");
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
	dprintf(trace_file_fd, "function night_http_process_request_headers:\treturn\n\n");
}

int
night_http_alloc_large_header_buffer(night_http_request_t *r, int request_line)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"function:\t" "night_http_alloc_large_header_buffer\n\n");
    
    int rc;
    
    return rc;
}

void
night_http_run_posted_requests(night_connection_t *c)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"function:\t" "night_http_run_posted_requests\n\n");
    
	night_http_request_t         *r;
    night_http_posted_request_t  *pr;
    
    for ( ;; ) 
    {
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
		dprintf(trace_file_fd, "for ( ;; )\n\n");
		
		// 如果连接 c 已被销毁（destroyed 为 1），立即退出函数
        if (c->destroyed) 
        {	
        	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        	dprintf(trace_file_fd, "连接已被销毁\n\n");
        	
        	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        	dprintf(trace_file_fd, "function night_http_run_posted_requests:\treturn\n\n");
        	
            return;
        }
        
		// 从连接 c 的 data 字段中获取关联的 HTTP 请求指针
        r = c->data;
        
		// 获取主请求（r->main）的挂起请求链表头
        // 每个主请求（main request）维护一个 posted_requests 链表，用于存放需要“稍后处理”的子请求或自身
        pr = r->main->posted_requests;
        
		//如果挂起请求链表为空，说明没有待处理的请求，函数直接返回
        if (pr == NULL) 
        {	
        	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        	dprintf(trace_file_fd, "没有待处理的请求\n\n");
        	
        	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        	dprintf(trace_file_fd, "function night_http_run_posted_requests:\treturn\n\n");
        	
            return;
        }
        
		// 将链表头指针前移：从链表中移除当前要处理的节点 pr
        r->main->posted_requests = pr->next;

		// 从挂起请求节点 pr 中取出实际要处理的 HTTP 请求 r
        r = pr->request;
        
		// 调用该请求的 写事件处理器
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
		dprintf(trace_file_fd, "r->write_event_handler(r)\n\n");
		
        r->write_event_handler(r);
        
        dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        dprintf(trace_file_fd, "r->write_event_handler(r) completed\n\n");
        
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
		dprintf(trace_file_fd, "function night_http_run_posted_requests:\treturn\n\n");
    }
}

int
night_http_process_host(night_http_request_t *r, night_table_elt_t *h, size_t offset)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"function:\t" "night_http_process_host\n\n");
    
    // host value
    night_str_t  				host;
    int							rc;
    
    // 检查是否已经存在一个 Host 头
    // 如果非 NULL，说明客户端发送了多个 Host 头（HTTP/1.1 规范禁止重复 Host 头）
	if (r->headers_in.host) 
	{
		dprintf(error_log_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
		dprintf(error_log_fd, "client sent duplicate host header\n\n");
		
		//  终止当前请求，并返回 HTTP 400（Bad Request）
        night_http_finalize_request(r, NIGHT_HTTP_BAD_REQUEST);
        return NIGHT_ERROR;
    }
    
    // 将当前 Host 头 h 保存到请求结构体中。
	// r->headers_in.host = h：记录 Host 头指针。
	// h->next = NULL：确保链表断开（防止后续误操作，因为 Host 只能有一个）
	r->headers_in.host = h;
    h->next = NULL;
    
    // 将 Host 头的值（字符串）赋给局部变量 host，用于后续验证
    host = h->value;
    
    // 验证 Host 值的合法性
    rc = night_http_validate_host(&host, r->pool, 0);
    
    // 如果 Host 格式非法（但非系统错误）：
	if (rc == NIGHT_DECLINED) 
	{
		dprintf(error_log_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
		dprintf(error_log_fd, "client sent invalid host header\n\n");
		// 返回 400 Bad Request
        night_http_finalize_request(r, NIGHT_HTTP_BAD_REQUEST);
        return NIGHT_ERROR;
    }

	// 如果是内部错误（如内存不足）：
    if (rc == NIGHT_ERROR) 
    {
    	// 关闭请求，返回 500 Internal Server Error
    	// 注意：这里用的是 close_request 而不是 finalize_request，可能因为此时请求尚未完全建立，不能走正常响应流程。
		// 返回 NIGHT_ERROR
        night_http_close_request(r, NIGHT_HTTP_INTERNAL_SERVER_ERROR);
        return NIGHT_ERROR;
    }
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd, "return NIGHT_OK;\n\n");
    
    return NIGHT_OK;
}

int
night_http_process_request_header(night_http_request_t *r)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"function:\t" "night_http_process_request_header\n\n");
    
    // HTTP/1.1 必须包含 Host 头
	if (r->headers_in.host == NULL && r->http_version > NIGHT_HTTP_VERSION_10) 
	{
		dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
		dprintf(trace_file_fd,"client sent HTTP/1.1 request without \"Host\" header" );
                   
        // 结束当前请求，返回 400 状态码
        night_http_finalize_request(r, NIGHT_HTTP_BAD_REQUEST);
        return NIGHT_ERROR;
    }
    
    // 解析 Content-Length
	if (r->headers_in.content_length) 
	{
		// 将字符串转换为 off_t 类型的整数（注意：不是 atoi，而是支持大整数的 ato off_t）
        r->headers_in.content_length_n = night_atoof(r->headers_in.content_length->value.data, r->headers_in.content_length->value.len);
		
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
		dprintf(trace_file_fd, "content_length_n=%ld\n\n", r->headers_in.content_length_n);
		
        if (r->headers_in.content_length_n == NIGHT_ERROR) 
        {
        	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        	dprintf(trace_file_fd, "client sent invalid \"Content-Length\" header\n\n");
        	
            night_http_finalize_request(r, NIGHT_HTTP_BAD_REQUEST);
            return NIGHT_ERROR;
        }
    }
    
    // 处理 Transfer-Encoding
    if (r->headers_in.transfer_encoding) 
    {
    	// Transfer-Encoding 是 HTTP/1.1 引入的特性，HTTP/1.0 不支持。
		// 若 HTTP/1.0 请求包含此头，视为非法，返回 400
		if (r->http_version < NIGHT_HTTP_VERSION_11) 
		{
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
			dprintf(trace_file_fd, "client sent HTTP/1.0 request with \"Transfer-Encoding\" header\n\n");

            night_http_finalize_request(r, NIGHT_HTTP_BAD_REQUEST);
            return NIGHT_ERROR;
        }
        
        // "chunked" 编码
		if (r->headers_in.transfer_encoding->value.len == 7 
			&& night_strncasecmp(r->headers_in.transfer_encoding->value.data, (char *) "chunked", 7) == 0)
        {
        	// 不能同时有 Content-Length 和 Transfer-Encoding
            if (r->headers_in.content_length) 
            {
            	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
            	dprintf(trace_file_fd, "client sent \"Content-Length\" and \"Transfer-Encoding\" headers at the same time\n\n");
            	
                night_http_finalize_request(r, NIGHT_HTTP_BAD_REQUEST);
                return NIGHT_ERROR;
            }
			// 设置 chunked 标志
            r->headers_in.chunked = 1;

        } 
        // 不支持其他 Transfer-Encoding
        else 
        {	
        	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        	dprintf(trace_file_fd, "client sent unknown \"Transfer-Encoding\":\"");
        	write(trace_file_fd, r->headers_in.transfer_encoding->value.data, r->headers_in.transfer_encoding->value.len);
        	dprintf(trace_file_fd, "\"\n\n");
        	
            night_http_finalize_request(r, NIGHT_HTTP_NOT_IMPLEMENTED);
            return NIGHT_ERROR;
        }
	}
	
	// Keep-Alive 超时处理
	if (r->headers_in.connection_type == NIGHT_HTTP_CONNECTION_KEEP_ALIVE) 
	{
        if (r->headers_in.keep_alive) 
        {
            r->headers_in.keep_alive_n = night_atotm(r->headers_in.keep_alive->value.data, r->headers_in.keep_alive->value.len);
        }
    }
    
    // 禁止 CONNECT 方法
	if (r->method == NIGHT_HTTP_CONNECT) 
	{
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
		dprintf(trace_file_fd, "client sent CONNECT method\n\n");
		
		// 返回 405 Method Not Allowed
        night_http_finalize_request(r, NIGHT_HTTP_NOT_ALLOWED);
        return NIGHT_ERROR;
    }
    
    // 禁止 TRACE 方法
	if (r->method == NIGHT_HTTP_TRACE) 
	{
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
		dprintf(trace_file_fd, "client sent TRACE method\n\n");
		
        night_http_finalize_request(r, NIGHT_HTTP_NOT_ALLOWED);
        return NIGHT_ERROR;
    }
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd, "return NIGHT_OK;\n\n");
    
    return NIGHT_OK;
}

void
night_http_process_request(night_http_request_t *r)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"function:\t" "night_http_process_request\n\n");
    
	night_connection_t  *c;

    c = r->connection;
    
    // 如果读事件（接收请求头）设置了超时定时器，则删除它。
	// 意义：请求头已接收完毕，不再需要“读取超时”定时器
	if (c->read->timer_set) 
	{
		dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
		dprintf(trace_file_fd,"if (c->read->timer_set)\n\n");
		
        night_event_del_timer(c->read);
    }
    
	// 将连接的读写事件处理函数统一设置为 night_http_request_handler
	// 意义：此后所有 I/O 事件（如读取请求体、发送响应）都由该通用 handler 处理，它会根据请求状态调用 r->read_event_handler 或 r->write_event_handler
	c->read->handler = 	night_http_request_handler;
    c->write->handler = night_http_request_handler;
    
    // 设置请求的读事件处理函数为 night_http_block_reading
	// 意义：在正常处理阶段，通常不需要再读取数据（除非有请求体），所以使用“空操作”handler 避免意外读取。
    r->read_event_handler = night_http_block_reading;
	
	// 进入 HTTP 请求处理主流程
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
	dprintf(trace_file_fd,"night_http_handler(r)\n\n");
	
    night_http_handler(r);
    
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"night_http_handler(r) completed\n\n");
    
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"function night_http_process_request:\treturn\n\n");
    
    return;
}

int
night_http_process_connection(night_http_request_t *r, night_table_elt_t *h, size_t offset)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
	dprintf(trace_file_fd,"function:\t" "night_http_process_connection\n\n");
	
	// 调用通用头部处理函数,用于将当前头部字段 h 存入 r->headers_in 结构体中对应的位置。
    if (night_http_process_header_line(r, h, offset) != NIGHT_OK) 
    {
        return NIGHT_ERROR;
    }

	// 检查是否包含 "close"
	// 如果 Connection 头部的值中包含子串(不区分大小写) "close"，则将 r->headers_in.connection_type 设置为 NIGHT_HTTP_CONNECTION_CLOSE
	// 表示客户端希望在本次请求/响应完成后关闭连接。
	// 后续会据此决定是否在响应后关闭 TCP 连接。
    if (night_strcasestrn(h->value.data, "close", 5)) 
    {
    	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    	dprintf(trace_file_fd,"request connection header include close\n\n");
    	
        r->headers_in.connection_type = NIGHT_HTTP_CONNECTION_CLOSE;

    }
    // 否则检查是否包含 "keep-alive"
    else if (night_strcasestrn(h->value.data, "keep-alive", 10)) 
    {
		dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    	dprintf(trace_file_fd,"request connection header include keep-alive\n\n");
    	
        r->headers_in.connection_type = NIGHT_HTTP_CONNECTION_KEEP_ALIVE;
    }

    return NIGHT_OK;
}

int
night_http_process_header_line(night_http_request_t *r, night_table_elt_t *h, size_t offset)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
	dprintf(trace_file_fd,"function:\t" "night_http_process_header_line\n\n");
	
    night_table_elt_t  **ph;

    ph = (night_table_elt_t **) ((char *) &r->headers_in + offset);

    while (*ph) 
    {
    	ph = &(*ph)->next; 
    }

    *ph = h;
    h->next = NULL;

    return NIGHT_OK;
}

int
night_http_process_user_agent(night_http_request_t *r, night_table_elt_t *h, size_t offset)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
	dprintf(trace_file_fd,"function:\t" "night_http_process_user_agent\n\n");
	
	// user agent value
	char  		*user_agent;
	// position of msie int user_agent
	char		*msie;
	
	// 将当前头字段（h）存入 r->headers_in 结构体中对应的位置（由 offset 指定）
	if (night_http_process_header_line(r, h, offset) != NIGHT_OK) 
	{
        return NIGHT_ERROR;
    }
    
    // 将 user_agent 指针指向 User-Agent 头的实际字符串数据
    user_agent = h->value.data;
    
    // 在 user_agent 字符串中查找子串 "MSIE "（注意末尾有空格）
    msie = night_strstrn(user_agent, "MSIE ", 5);
    
    // 判断是否成功找到 "MSIE "，并且其后至少还有 7 个字符（即 "MSIE X.Y" 至少需要 7 字节：如："MSIE 6.0" ）
    if (msie && msie + 7 < user_agent + h->value.len) 
    {
    	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    	dprintf(trace_file_fd,"MSIE\n\n");
    	
    	// 标记当前请求来自  IE 浏览器 Internet Explorer（MSIE）
    	r->headers_in.msie = 1;
    	
    	// 检查版本号格式是否为 X.Y（即主版本后紧跟小数点）
		if (msie[6] == '.') 
		{
            switch (msie[5]) 
            {
            	case '4':
            	case '5':
            		// msie6 标志不仅表示 IE6，还包括 IE4/5，因为它们有类似的兼容性问题
                	r->headers_in.msie6 = 1;
                	break;
            	case '6':
            		// "SV1" 是 IE6 SP2（或更高）的标识，表示“安全版本 1”，修复了某些 bug。
            		// 如果找到 "SV1"，则 不设置 msie6，因为该版本行为更标准
                	if (night_strstrn(msie + 8, "SV1", 3) == NULL) 
                	{
                    	r->headers_in.msie6 = 1;
                	}
                	break;
            }
        }
    }
    
    // 检测 Opera 浏览器
	if (night_strstrn(user_agent, "Opera", 5)) 
	{
		dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
		dprintf(trace_file_fd,"Opera\n\n");
		
        r->headers_in.opera = 1;
        r->headers_in.msie = 0;
        r->headers_in.msie6 = 0;
    }
    
    // 只有当 既不是 IE 也不是 Opera 时，才继续检测其他现代浏览器。
	if (!r->headers_in.msie && !r->headers_in.opera) 
	{
		// 检测基于 Gecko 引擎的浏览器（如 Firefox）
        if (night_strstrn(user_agent, "Gecko/", 6)) 
        {
			dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
			dprintf(trace_file_fd,"Gecko\n\n");
		
            r->headers_in.gecko = 1;

        }
        // 检测 Chrome 浏览器 
        else if (night_strstrn(user_agent, "Chrome/", 7 - 1)) 
        {
			dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
			dprintf(trace_file_fd,"Chrome\n\n");
			
            r->headers_in.chrome = 1;

        } 
        else if (night_strstrn(user_agent, "Safari/", 7) && night_strstrn(user_agent, "Mac OS X", 8 - 1))
        {
			dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
			dprintf(trace_file_fd,"Safari\n\n");
			
            r->headers_in.safari = 1;

        } 
        else if (night_strstrn(user_agent, "Konqueror", 9)) 
        {
			dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
			dprintf(trace_file_fd,"Konqueror\n\n");
        
            r->headers_in.konqueror = 1;
        }
    }
    
	return NIGHT_OK;
}

// 该函数用于“阻塞”当前连接上的读事件，即告诉事件驱动系统：暂时不需要再监听该连接上的可读事件。这通常发生在 HTTP 请求已经读取完毕、或者后续处理不再需要读取客户端数据
void
night_http_block_reading(night_http_request_t *r)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
	dprintf(trace_file_fd, "function:\t" "night_http_block_reading\n");
	dprintf(trace_file_fd, "http reading blocked\n\n");

}

void
night_http_request_handler(night_event_t *ev)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
	dprintf(trace_file_fd, "function:\t" "night_http_request_handler\n");
	
	return;
}
// HTTP 请求事件的主处理函数（事件回调函数），用于处理客户端连接上的读/写事件
/*
void
night_http_request_handler(night_event_t *ev)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
	dprintf(trace_file_fd, "function:\t" "night_http_request_handler\n");
	
    night_connection_t		*c;
    night_http_request_t	*r;

    c = ev->data;
    r = c->data;

	// 检查连接是否被标记为需要关闭（c->close 为真）
    if (c->close) 
    {
    	// 对主请求（main）的引用计数加一。这是为了防止在终止过程中请求被过早释放
        r->main->count++;
        // 终止当前请求的处理，清理资源，可能发送响应或直接关闭。
        night_http_terminate_request(r, 0);
        // 执行该连接上所有“已挂起”（posted）的请求
        night_http_run_posted_requests(c);
        // 提前返回，不再继续处理读/写事件。
        return;
    }

	// 作用：清除事件的 delayed 和 timedout 标志。
	// ev->delayed：表示该事件因限速（如 sendfile 限速、写缓冲区满）被延迟处理。
	// ev->timedout：表示事件因超时被触发（例如写超时）。
	// 逻辑：如果事件同时被标记为“延迟”和“超时”，说明延迟已结束（超时作为延迟结束的信号），现在可以清除这两个标志，恢复正常处理。
	// 意义：确保事件状态正确，避免后续逻辑误判为仍处于延迟或超时状态。
	// 注：这通常出现在限速（rate limiting）或异步 I/O 场景中，例如使用 ngx_linux_sendfile_chain 时。 
    if (ev->delayed && ev->timedout) 
    {
        ev->delayed = 0;
        ev->timedout = 0;
    }

    if (ev->write) {
        r->write_event_handler(r);

    } else {
        r->read_event_handler(r);
    }

    ngx_http_run_posted_requests(c);
}
*/

void
night_http_request_empty_handler(night_http_request_t *r)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
	dprintf(trace_file_fd, "function:\t" "night_http_request_empty_handler\n");

    return;
}
