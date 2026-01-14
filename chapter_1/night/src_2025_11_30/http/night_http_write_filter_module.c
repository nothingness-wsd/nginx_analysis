#include "night_core.h"
#include "night_http_write_filter_module.h"
#include "night_http_module_ctx.h"
#include "night_module.h"
#include "night_http_module.h"
#include "night_connection.h"
#include "night_buf.h"

night_http_module_ctx_t night_http_write_filter_module_ctx =
{
	NULL,                                  		// create main configuration
	NULL,                                  		// create server configuration
	NULL,										// create location configuration
	night_http_write_filter_init				//	postconfiguration
};

night_module_t	night_http_write_filter_module =
{
    "night_http_write_filter_module",			// name;
    NIGHT_MODULE_UNSET_INDEX, 					// index;
    NIGHT_MODULE_UNSET_INDEX, 					// ctx_index;
    NIGHT_HTTP_MODULE, 							// type;
    NULL,										// module directives
    &night_http_write_filter_module_ctx,		// module context
    NULL,										// init_process
    NULL,										// exit_process
    NULL										// exit_master
};


int
night_http_write_filter_init(night_conf_t *cf)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_http_write_filter_init\n\n");
	
    night_http_top_body_filter = night_http_write_filter;

    return NIGHT_OK;
}

/*
HTTP 模块的 写过滤器（write filter），负责将 HTTP 响应的数据缓冲链（ngx_chain_t）发送给客户端。
这是 Nginx HTTP 输出链中的核心环节，负责合并缓冲、限速、发送数据、管理事件状态等。
*/
int
night_http_write_filter(night_http_request_t *r, night_chain_t *in)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_http_write_filter\n\n");
	
	night_connection_t          *c;
	
	// size：总待发送字节数
	off_t                      	size;
	
	size_t						limit;
	
	size_t						sent;
	
	// 标志位
	int							flush;
	int							sync;
	int							last;
	
	// 链表操作指针
	night_chain_t               **ll;
	night_chain_t               *cl, *ln;
	
	night_http_core_loc_conf_t	*clcf;
	
	night_chain_t				*chain;
	
	// 获取连接
    c = r->connection;

    if (c->error) 
    {
        return NIGHT_ERROR;
    }
    
	// 初始化状态
    size = 0;
    flush = 0;
    sync = 0;
    last = 0;
    ll = &r->out;
    
	// 遍历已存在的 r->out 链（旧数据）
	// 遍历当前已缓冲但未发送的数据链 r->out
    for (cl = r->out; cl; cl = cl->next) 
    {
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "for (cl = r->out; cl; cl = cl->next)\n\n");
    	
    	//ll 指向最后一个节点的 next，为后续追加做准备
    	ll = &cl->next;
    	
    	// 关键检查：普通缓冲区大小不能为 0 或负数（特殊缓冲区如 flush/sync/last 除外）。
    	if (night_buf_size(cl->buf) <= 0)
    	{
    		return NIGHT_ERROR;
    	}
    	
    	size += night_buf_size(cl->buf);
    	
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(trace_file_fd, "size=%ld\n\n", size);
        
		if (cl->buf->flush || cl->buf->recycled) 
		{
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(trace_file_fd, "if (cl->buf->flush || cl->buf->recycled)\nflush = 1;\n\n");
			
            flush = 1;
        }

        if (cl->buf->sync) 
        {
        	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "if (cl->buf->sync)\nsync = 1;\n\n");
        	
            sync = 1;
        }

        if (cl->buf->last_buf) 
        {
        	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "if (cl->buf->last_buf)\nlast = 1;\n");
        	
            last = 1;
        }
    }
    
    	// 将新输入链 in 追加到 r->out
    for (ln = in; ln; ln = ln->next) 
    {	
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "for (ln = in; ln; ln = ln->next)\n\n");
    	
		// 为 in 中每个节点分配新的链表节点（浅拷贝 buf 指针）。
		// 将新节点追加到 r->out 末尾。
		// 更新 ll 指向新尾部。
        cl = night_pmalloc_chain_link(r->pool);
        if (cl == NULL) 
        {
            return NIGHT_ERROR;
        }

        cl->buf = ln->buf;
        *ll = cl;
        ll = &cl->next;
        
        if (night_buf_size(cl->buf) <= 0)
        {
        	return NIGHT_ERROR;
        }
        
        // 累加总大小
        size += night_buf_size(cl->buf);
        
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(trace_file_fd, "size=%ld\n\n", size);
        
		if (cl->buf->flush || cl->buf->recycled) 
		{
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(trace_file_fd, "if (cl->buf->flush || cl->buf->recycled)\nflush = 1;\n\n");
			
            flush = 1;
        }

        if (cl->buf->sync) 
        {
        	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "if (cl->buf->sync)\nsync = 1;\n\n");
        	
            sync = 1;
        }

        if (cl->buf->last_buf) 
        {
        	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "if (cl->buf->last_buf)\nlast = 1;\n\n");
        	
            last = 1;
        }
    }
    
    *ll = NULL;
    
    //clcf = r->loc_conf[night_http_core_module.index_ctx];
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "size=%ld\n\n", size);
    
    // 如果没有结束标志、没有强制刷新、且总大小小于 postpone_output（默认 1460 字节），则暂不发送，等待更多数据合并（减少小包）
	// 直接返回 NGX_OK，数据保留在 r->out 中
	if (!last && !flush && in && size < 1460) 
	{
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "return NIGHT_OK;\n\n");
		
        return NIGHT_OK;
    }
    
    limit = 2097152;
    
	// 记录发送前的已发送字节数
    sent = c->sent;
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "sent=%ld\n\n", sent);
    
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "chain = c->send_chain(c, r->out, limit);\n\n");
    
	// 调用底层发送函数
    chain = c->send_chain(c, r->out, limit);
    
	// 如果发送失败，设置连接错误标志并返回错误
    if (chain == NIGHT_CHAIN_ERROR) 
    {
        c->error = 1;
        return NIGHT_ERROR;
    }
    
	// 释放已经发送完的链表节点
    for (cl = r->out; cl && cl != chain; /* void */) 
    {	
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "night_free_chain(r->pool, ln);\n\n");
    	
        ln = cl;
        cl = cl->next;
        night_free_chain(r->pool, ln);
    }
    
    // 更新输出链表为未发送完的部分
	r->out = chain;
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "r->out=%p\n\n", r->out);
    
    // 如果还有数据未发送完，设置缓冲标志并返回AGAIN
	if (chain) 
	{   
		// c->buffered |= NGX_HTTP_WRITE_BUFFERED;
        dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(trace_file_fd, "if (chain) {\nreturn NIGHT_AGAIN;\n\n");
        
        return NIGHT_AGAIN;
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "return NIGHT_OK;\n\n");
     
	return NIGHT_OK;
};	


