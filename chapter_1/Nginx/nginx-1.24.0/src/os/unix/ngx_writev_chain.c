
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>


ngx_chain_t *
ngx_writev_chain(ngx_connection_t *c, ngx_chain_t *in, off_t limit)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "ngx_writev_chain\n\n");
	
    ssize_t        n, sent;
    off_t          send, prev_send;
    ngx_chain_t   *cl;
    ngx_event_t   *wev;
    ngx_iovec_t    vec;
    struct iovec   iovs[NGX_IOVS_PREALLOCATE];

    wev = c->write;

    if (!wev->ready) {
        return in;
    }

#if (NGX_HAVE_KQUEUE)

    if ((ngx_event_flags & NGX_USE_KQUEUE_EVENT) && wev->pending_eof) {
        (void) ngx_connection_error(c, wev->kq_errno,
                               "kevent() reported about an closed connection");
        wev->error = 1;
        return NGX_CHAIN_ERROR;
    }

#endif

    /* the maximum limit size is the maximum size_t value - the page size */

    if (limit == 0 || limit > (off_t) (NGX_MAX_SIZE_T_VALUE - ngx_pagesize)) {
        limit = NGX_MAX_SIZE_T_VALUE - ngx_pagesize;
    }

    send = 0;

    vec.iovs = iovs;
    vec.nalloc = NGX_IOVS_PREALLOCATE;

    for ( ;; ) {
        prev_send = send;

        /* create the iovec and coalesce the neighbouring bufs */

        cl = ngx_output_chain_to_iovec(&vec, in, limit - send, c->log);

        if (cl == NGX_CHAIN_ERROR) {
            return NGX_CHAIN_ERROR;
        }

        if (cl && cl->buf->in_file) {
            ngx_log_error(NGX_LOG_ALERT, c->log, 0,
                          "file buf in writev "
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

            return NGX_CHAIN_ERROR;
        }

        send += vec.size;

        n = ngx_writev(c, &vec);

        if (n == NGX_ERROR) {
            return NGX_CHAIN_ERROR;
        }

        sent = (n == NGX_AGAIN) ? 0 : n;

        c->sent += sent;

        in = ngx_chain_update_sent(in, sent);

        if (send - prev_send != sent) {
            wev->ready = 0;
            return in;
        }

        if (send >= limit || in == NULL) {
            return in;
        }
    }
}

/*
将ngx_chain_t链转换为iovec数组，返回处理完的链表节点指针
*/
ngx_chain_t *
ngx_output_chain_to_iovec(ngx_iovec_t *vec, ngx_chain_t *in, size_t limit,
    ngx_log_t *log)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "ngx_output_chain_to_iovec\n\n");
	
    size_t         total, size;
    u_char        *prev;
    ngx_uint_t     n;
    struct iovec  *iov;
    
    ssize_t			temp;

    iov = NULL;
    prev = NULL;
    total = 0;
    n = 0;

	// 主循环：遍历chain链表，条件是链表节点存在且总大小未超过限制
    for ( /* void */ ; in && total < limit; in = in->next) 
    {
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "for ( /* void */ ; in && total < limit; in = in->next)\n\n");

		// 跳过特殊buf：如果当前buf是特殊类型的，跳过处理
        if (ngx_buf_special(in->buf)) 
        {
        	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "if (ngx_buf_special(in->buf))\n\n");
        	
            continue;
        }

		// 文件buf处理：如果buf在文件中，停止处理（因为不能直接映射到内存iovec）
        if (in->buf->in_file) 
        {
        	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "if (in->buf->in_file)\nbreak;\n\n");
        	
            break;
        }
		
		// 检查buf是否在内存中：如果不是内存buf，记录错误
        if (!ngx_buf_in_memory(in->buf)) 
        {
        	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "if (!ngx_buf_in_memory(in->buf))\n\n");
        	
            ngx_log_error(NGX_LOG_ALERT, log, 0,
                          "bad buf in output chain "
                          "t:%d r:%d f:%d %p %p-%p %p %O-%O",
                          in->buf->temporary,
                          in->buf->recycled,
                          in->buf->in_file,
                          in->buf->start,
                          in->buf->pos,
                          in->buf->last,
                          in->buf->file,
                          in->buf->file_pos,
                          in->buf->file_last);

            ngx_debug_point();

            return NGX_CHAIN_ERROR;
        }

		// 计算当前buf的有效数据大小
        size = in->buf->last - in->buf->pos;
        
        dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(trace_file_fd, "size=%ld\n\n", size);
        
        dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        temp = write(trace_file_fd, in->buf->pos, size);
        dprintf(trace_file_fd, "temp=%ld\n\n", temp);

		// 限制大小：如果当前buf大小超过剩余限制，截取到限制范围内
        if (size > limit - total) 
        {
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "if (size > limit - total)\n");
        	
            size = limit - total;
            
            dprintf(trace_file_fd, "size(%ld)=limit(%ld) - total(%ld)\n\n", size, limit, total);
        }

		// 内存连续性检查：如果当前buf与前一个buf地址连续，合并到同一个iovec项中（合并相邻的连续内存块以减少iovec数量）
        if (prev == in->buf->pos) 
        {
            iov->iov_len += size;
        } 
        // 新建iovec项：
		// 检查iovec数组是否还有空间
		// 获取新的iovec项
		// 设置基地址和长度
        else 
        {
            if (n == vec->nalloc) {
                break;
            }

            iov = &vec->iovs[n++];

            iov->iov_base = (void *) in->buf->pos;
            iov->iov_len = size;
        }

        prev = in->buf->pos + size;
        total += size;
    }

    vec->count = n;
    vec->size = total;

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "vec->count=%ld\n", vec->count);
	dprintf(trace_file_fd, "return in=%p\n\n", in);
	
    return in;
}


ssize_t
ngx_writev(ngx_connection_t *c, ngx_iovec_t *vec)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "ngx_writev\n\n");

    ssize_t    n;
    ngx_err_t  err;

eintr:
/*
执行系统调用 writev：
c->fd：连接的文件描述符
vec->iovs：iovec数组指针（包含多个缓冲区地址和长度）
vec->count：iovec数组中的元素个数
writev 系统调用可以一次写入多个不连续的缓冲区数据
*/
    n = writev(c->fd, vec->iovs, vec->count);

    ngx_log_debug2(NGX_LOG_DEBUG_EVENT, c->log, 0,
                   "writev: %z of %uz", n, vec->size);

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "n=%ld\n\n", n);
	
	// 检查 writev 系统调用是否失败（返回 -1 表示失败）
    if (n == -1) 
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "if (n == -1)\n\n");
    	
        err = ngx_errno;

        switch (err) {
        case NGX_EAGAIN:
            ngx_log_debug0(NGX_LOG_DEBUG_EVENT, c->log, err,
                           "writev() not ready");
            return NGX_AGAIN;

        case NGX_EINTR:
            ngx_log_debug0(NGX_LOG_DEBUG_EVENT, c->log, err,
                           "writev() was interrupted");
            goto eintr;

        default:
            c->write->error = 1;
            ngx_connection_error(c, err, "writev() failed");
            return NGX_ERROR;
        }
    }

    return n;
}
