
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


static ngx_int_t ngx_http_write_filter_init(ngx_conf_t *cf);


static ngx_http_module_t  ngx_http_write_filter_module_ctx = {
    NULL,                                  /* preconfiguration */
    ngx_http_write_filter_init,            /* postconfiguration */

    NULL,                                  /* create main configuration */
    NULL,                                  /* init main configuration */

    NULL,                                  /* create server configuration */
    NULL,                                  /* merge server configuration */

    NULL,                                  /* create location configuration */
    NULL,                                  /* merge location configuration */
};


ngx_module_t  ngx_http_write_filter_module = {
    NGX_MODULE_V1,
    &ngx_http_write_filter_module_ctx,     /* module context */
    NULL,                                  /* module directives */
    NGX_HTTP_MODULE,                       /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};

/*
 HTTP 模块的 写过滤器（write filter），负责将 HTTP 响应的数据缓冲链（ngx_chain_t）发送给客户端。
 这是 Nginx HTTP 输出链中的核心环节，负责合并缓冲、限速、发送数据、管理事件状态等。
*/
ngx_int_t
ngx_http_write_filter(ngx_http_request_t *r, ngx_chain_t *in)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "ngx_http_write_filter\n\n");
	
    off_t                      size, sent, nsent, limit;
    ngx_uint_t                 last, flush, sync;
    ngx_msec_t                 delay;
    ngx_chain_t               *cl, *ln, **ll, *chain;
    ngx_connection_t          *c;
    ngx_http_core_loc_conf_t  *clcf;

    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "in->buf->in_file=%d\n\n", in->buf->in_file);
    
	// 获取连接
    c = r->connection;

    if (c->error) {
        return NGX_ERROR;
    }

	// 初始化状态
    size = 0;
    flush = 0;
    sync = 0;
    last = 0;
    ll = &r->out;
    
    if(r->out == NULL)
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "r->out=NULL\n\n");
    }

    /* find the size, the flush point and the last link of the saved chain */
	// 遍历已存在的 r->out 链（旧数据）
    for (cl = r->out; cl; cl = cl->next) {
    
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "for (cl = r->out; cl; cl = cl->next)\n\n");
    	
        ll = &cl->next;

        ngx_log_debug7(NGX_LOG_DEBUG_EVENT, c->log, 0,
                       "write old buf t:%d f:%d %p, pos %p, size: %z "
                       "file: %O, size: %O",
                       cl->buf->temporary, cl->buf->in_file,
                       cl->buf->start, cl->buf->pos,
                       cl->buf->last - cl->buf->pos,
                       cl->buf->file_pos,
                       cl->buf->file_last - cl->buf->file_pos);
		
		// 验证缓冲区大小
		// 关键检查：普通缓冲区大小不能为 0 或负数（特殊缓冲区如 flush/sync/last 除外）。
		// 若违反，说明模块逻辑错误，触发 debug_point()（调试断点）并返回错误。
        if (ngx_buf_size(cl->buf) == 0 && !ngx_buf_special(cl->buf)) {
            ngx_log_error(NGX_LOG_ALERT, c->log, 0,
                          "zero size buf in writer "
                          "t:%d r:%d f:%d %p %p-%p %p %O-%O",
                          cl->buf->temporary,
                          cl->buf->recycled,
                          cl->buf->in_file,
                          cl->buf->start,
                          cl->buf->pos,
                          cl->buf->last,
                          cl->buf->file,
                          cl->buf->file_pos,
                          cl->buf->file_last);

            ngx_debug_point();
            return NGX_ERROR;
        }

        if (ngx_buf_size(cl->buf) < 0) {
            ngx_log_error(NGX_LOG_ALERT, c->log, 0,
                          "negative size buf in writer "
                          "t:%d r:%d f:%d %p %p-%p %p %O-%O",
                          cl->buf->temporary,
                          cl->buf->recycled,
                          cl->buf->in_file,
                          cl->buf->start,
                          cl->buf->pos,
                          cl->buf->last,
                          cl->buf->file,
                          cl->buf->file_pos,
                          cl->buf->file_last);

            ngx_debug_point();
            return NGX_ERROR;
        }

        size += ngx_buf_size(cl->buf);
        
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

    /* add the new chain to the existent one */
	// 将新输入链 in 追加到 r->out
    for (ln = in; ln; ln = ln->next) {
    	
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "for (ln = in; ln; ln = ln->next)\n\n");
    	
    	// 为 in 中每个节点分配新的链表节点（浅拷贝 buf 指针）。
		// 将新节点追加到 r->out 末尾。
		// 更新 ll 指向新尾部。
        cl = ngx_alloc_chain_link(r->pool);
        if (cl == NULL) {
            return NGX_ERROR;
        }

        cl->buf = ln->buf;
        *ll = cl;
        ll = &cl->next;

        ngx_log_debug7(NGX_LOG_DEBUG_EVENT, c->log, 0,
                       "write new buf t:%d f:%d %p, pos %p, size: %z "
                       "file: %O, size: %O",
                       cl->buf->temporary, cl->buf->in_file,
                       cl->buf->start, cl->buf->pos,
                       cl->buf->last - cl->buf->pos,
                       cl->buf->file_pos,
                       cl->buf->file_last - cl->buf->file_pos);

        if (ngx_buf_size(cl->buf) == 0 && !ngx_buf_special(cl->buf)) {
            ngx_log_error(NGX_LOG_ALERT, c->log, 0,
                          "zero size buf in writer "
                          "t:%d r:%d f:%d %p %p-%p %p %O-%O",
                          cl->buf->temporary,
                          cl->buf->recycled,
                          cl->buf->in_file,
                          cl->buf->start,
                          cl->buf->pos,
                          cl->buf->last,
                          cl->buf->file,
                          cl->buf->file_pos,
                          cl->buf->file_last);

            ngx_debug_point();
            return NGX_ERROR;
        }

        if (ngx_buf_size(cl->buf) < 0) {
            ngx_log_error(NGX_LOG_ALERT, c->log, 0,
                          "negative size buf in writer "
                          "t:%d r:%d f:%d %p %p-%p %p %O-%O",
                          cl->buf->temporary,
                          cl->buf->recycled,
                          cl->buf->in_file,
                          cl->buf->start,
                          cl->buf->pos,
                          cl->buf->last,
                          cl->buf->file,
                          cl->buf->file_pos,
                          cl->buf->file_last);

            ngx_debug_point();
            return NGX_ERROR;
        }

        size += ngx_buf_size(cl->buf);
        
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "cl->buf->in_file=%d\n", cl->buf->in_file);
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

    ngx_log_debug3(NGX_LOG_DEBUG_HTTP, c->log, 0,
                   "http write filter: l:%ui f:%ui s:%O", last, flush, size);

    clcf = ngx_http_get_module_loc_conf(r, ngx_http_core_module);

    /*
     * avoid the output if there are no last buf, no flush point,
     * there are the incoming bufs and the size of all bufs
     * is smaller than "postpone_output" directive
     */
     
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "size=%ld\n\n", size);

    if (!last && !flush && in && size < (off_t) clcf->postpone_output) {
    
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "clcf->postpone_output=%ld\n", clcf->postpone_output);
    	dprintf(trace_file_fd, "if (!last && !flush && in && size < (off_t) clcf->postpone_output)\nreturn NGX_OK;\n\n");
    	
        return NGX_OK;
    }

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "c->write->delayed=%d\n\n", c->write->delayed);
	// 检查写事件是否被延迟（限速中）
	// 若写事件被限速延迟，则标记连接为“有 HTTP 数据待写”，返回 NGX_AGAIN，等待定时器触发
    if (c->write->delayed) 
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "if (c->write->delayed)\nreturn NGX_AGAIN;\n\n");
    	
        c->buffered |= NGX_HTTP_WRITE_BUFFERED;
        return NGX_AGAIN;
    }

	// 处理空输出链的特殊情况
    if (size == 0
        && !(c->buffered & NGX_LOWLEVEL_BUFFERED)
        && !(last && c->need_last_buf)
        && !(flush && c->need_flush_buf))
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "if (size == 0\t\n\n");
    	
        if (last || flush || sync) {
            for (cl = r->out; cl; /* void */) {
                ln = cl;
                cl = cl->next;
                ngx_free_chain(r->pool, ln);
            }

            r->out = NULL;
            c->buffered &= ~NGX_HTTP_WRITE_BUFFERED;

            return NGX_OK;
        }

        ngx_log_error(NGX_LOG_ALERT, c->log, 0,
                      "the http output chain is empty");

        ngx_debug_point();

        return NGX_ERROR;
    }

	// 处理限速（limit_rate）
	
    if (!r->limit_rate_set) 
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "if (!r->limit_rate_set)\n\n");
    	
        r->limit_rate = ngx_http_complex_value_size(r, clcf->limit_rate, 0);
        
        dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(trace_file_fd, "r->limit_rate=%ld\n\n", r->limit_rate);
        
        r->limit_rate_set = 1;
    }

    if (r->limit_rate) 
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "if (r->limit_rate)\n\n");

        if (!r->limit_rate_after_set) {
            r->limit_rate_after = ngx_http_complex_value_size(r,
                                                    clcf->limit_rate_after, 0);
            r->limit_rate_after_set = 1;
        }

        limit = (off_t) r->limit_rate * (ngx_time() - r->start_sec + 1)
                - (c->sent - r->limit_rate_after);

        if (limit <= 0) {
            c->write->delayed = 1;
            delay = (ngx_msec_t) (- limit * 1000 / r->limit_rate + 1);
            ngx_add_timer(c->write, delay);

            c->buffered |= NGX_HTTP_WRITE_BUFFERED;

            return NGX_AGAIN;
        }

        if (clcf->sendfile_max_chunk
            && (off_t) clcf->sendfile_max_chunk < limit)
        {
            limit = clcf->sendfile_max_chunk;
        }

    }
     
/* 
无限制速时，仅受 sendfile_max_chunk (2097152, 2M)限制
设置本次调用 send_chain() 时允许发送的最大字节数
在未启用限速（limit_rate）的情况下，对单次写操作的数据量施加一个软性上限，主要用于优化系统性能和资源使用。
1. 防止单次 sendfile() 调用阻塞过久
sendfile() 是一个系统调用，用于高效地在内核空间传输文件（零拷贝）。
但如果一次 sendfile 发送一个非常大的文件（比如 1GB），该调用可能会长时间阻塞（尽管是内核态，但仍占用 CPU 和 I/O 资源）。
设置 sendfile_max_chunk 可将大文件拆分成多个小块（如每块 2MB）发送，让出 CPU 时间片给其他连接，提升并发响应能力。
2. 避免饿死（starvation）其他请求
Nginx 是单线程事件循环模型（每个 worker）
如果某个请求独占大量发送时间，其他请求可能长时间得不到处理。
通过限制单次发送量，强制事件循环在每次发送后有机会处理其他事件（如新连接、其他请求的读写等）。
3. 控制内存和内核缓冲区压力
虽然 sendfile 是零拷贝，但 TCP 发送缓冲区仍有容量限制。
一次性提交过大数据可能导致内核缓冲区积压、丢包或触发 TCP 拥塞控制。
分块发送有助于平滑流量、减少突发压力。
*/
    else 
    {
        limit = clcf->sendfile_max_chunk;
        
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "limit=%ld\n\n", limit);
    }

	// 记录发送前的已发送字节数
    sent = c->sent;
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "sent=%ld\n\n", sent);

    ngx_log_debug1(NGX_LOG_DEBUG_HTTP, c->log, 0,
                   "http write filter limit %O", limit);
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "chain = c->send_chain(c, r->out, limit);\n\n");
    
	// 调用底层发送函数
    chain = c->send_chain(c, r->out, limit);

    ngx_log_debug1(NGX_LOG_DEBUG_HTTP, c->log, 0,
                   "http write filter %p", chain);

	// 如果发送失败，设置连接错误标志并返回错误
    if (chain == NGX_CHAIN_ERROR) {
        c->error = 1;
        return NGX_ERROR;
    }

	// 发送后流量控制：计算发送字节数，如果超过限制则设置延迟
    if (r->limit_rate) {

        nsent = c->sent;

        if (r->limit_rate_after) {
        
        	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "if (r->limit_rate_after) {\n\n");

            sent -= r->limit_rate_after;
            if (sent < 0) {
                sent = 0;
            }

            nsent -= r->limit_rate_after;
            if (nsent < 0) {
                nsent = 0;
            }
        }

        delay = (ngx_msec_t) ((nsent - sent) * 1000 / r->limit_rate);

        if (delay > 0) {
        	
        	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "if (delay > 0) {\n\n");
        	
            c->write->delayed = 1;
            ngx_add_timer(c->write, delay);
        }
    }

	// 如果还有数据未发送完且写就绪且未延迟，则将写事件添加到下一循环处理
    if (chain && c->write->ready && !c->write->delayed) {
    	
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "ngx_post_event\n\n");
    	
        ngx_post_event(c->write, &ngx_posted_next_events);
    }

	// 释放已经发送完的链表节点
    for (cl = r->out; cl && cl != chain; /* void */) {
    	
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "ngx_free_chain(r->pool, ln);\n\n");
    	
        ln = cl;
        cl = cl->next;
        ngx_free_chain(r->pool, ln);
    }

    r->out = chain;
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "r->out=%p\n\n", r->out);

	// 如果还有数据未发送完，设置缓冲标志并返回AGAIN
    if (chain) {
        c->buffered |= NGX_HTTP_WRITE_BUFFERED;
        
        dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(trace_file_fd, "if (chain) {\nreturn NGX_AGAIN;\n\n");
        
        return NGX_AGAIN;
    }

    c->buffered &= ~NGX_HTTP_WRITE_BUFFERED;

    if ((c->buffered & NGX_LOWLEVEL_BUFFERED) && r->postponed == NULL) {
        return NGX_AGAIN;
    }

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "return NGX_OK;\n\n");
	
    return NGX_OK;
}


static ngx_int_t
ngx_http_write_filter_init(ngx_conf_t *cf)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "ngx_http_write_filter_init\n\n");
	
    ngx_http_top_body_filter = ngx_http_write_filter;

    return NGX_OK;
}
