#include "night_core.h"
#include "night_http_connection.h"
#include "night_connection.h"
#include "night_pool.h"
#include "night_listening.h"
#include "night_http_core_module.h" 
#include "night_event.h"
#include "night_http_request.h"
#include "night_event_timer.h"

void
night_http_init_connection(night_connection_t *c)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"function:\t"  "night_http_init_connection\n\n");
    
    night_http_connection_t     *hc;
    night_listening_t			*ls;
    night_http_core_srv_conf_t	*scf;
    night_event_t				*rev,*wev;
    
    // 分配HTTP连接结构
	hc = night_pmalloc(c->pool, sizeof(night_http_connection_t));
    if (hc == NULL) 
    {
        night_http_close_connection(c);
        return;
    }
    
	//将HTTP连接结构指针保存到通用连接结构的data字段
	//这样后续可以从night_connection_t访问到HTTP特定的数据
    c->data = hc;
    
    ls = c->listening;
    hc->server = ls->server;
    
    scf = hc->server;
    
	//dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
 	//dprintf(trace_file_fd,"server_name=%s\n\n", scf->server_name.name.data);
 	
 	// 设置事件处理函数
	rev = c->read;
	wev = c->write;
	
    rev->handler = night_http_wait_request_handler;
    wev->handler = night_http_empty_handler;
    
    // 处理立即可读的连接
    if (rev->ready) 
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    	dprintf(trace_file_fd, "rev->handler(rev)\n\n" );
    	
		rev->handler(rev);
		
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
		dprintf(trace_file_fd, "rev->handler(rev) completed\n\n");
		
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
		dprintf(trace_file_fd, "function night_http_init_connection:\treturn\n\n");
		
        return;
    }
    
    night_event_add_timer(rev, scf->client_header_timeout);
    night_reusable_connection(c, 1);

    if (night_handle_read_event(rev) != NIGHT_OK) 
    {
        night_http_close_connection(c);
        return;
    }
    
}

void
night_http_close_connection(night_connection_t *c)
{
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"function:\t"  "night_http_close_connection\n\n");
    
	night_pool_t  *pool;
	
	c->destroyed = 1;
	pool = c->pool;
	
	night_close_connection(c);
	
	night_destroy_pool(pool);
         
}
