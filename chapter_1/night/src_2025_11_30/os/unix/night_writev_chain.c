#include "night_core.h"
#include "night_writev_chain.h"
#include "night_buf.h"
#include "night_connection.h"
#include "night_event.h"


night_chain_t*
night_writev_chain(night_connection_t *c, night_chain_t *in, off_t limit)
{
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd, "function:\t" "night_writev_chain\n\n" );
    
    return in;
}

night_chain_t *
night_output_chain_to_iovec(night_iovec_t *vec, night_chain_t *in, size_t limit)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_output_chain_to_iovec\n\n");
	
	size_t         	total, size;
    char        	*prev;
    int		     	n;
    struct iovec  	*iov;
    
    

	iov = NULL;
    prev = NULL;
    total = 0;
    n = 0;
    
    for ( /* void */ ; in && total < limit; in = in->next) 
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "for ( /* void */ ; in && total < limit; in = in->next)\n\n");
    	
		// 文件buf处理：如果buf在文件中，停止处理（因为不能直接映射到内存iovec）
        if (in->buf->in_file) 
        {
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "if (in->buf->in_file)\nbreak;\n");
        	
            break;
        }
        
		// 计算当前buf的有效数据大小
        size = in->buf->last - in->buf->pos;
        
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(trace_file_fd, "size=%ld\n\n", size);
        
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        write(trace_file_fd, in->buf->pos, size);
        dprintf(trace_file_fd, "\n\n");
        
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
            if (n == vec->nalloc) 
            {
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
night_writev(night_connection_t *c, night_iovec_t *vec)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_writev\n\n");

    ssize_t    n;

eintr:
/*
执行系统调用 writev：
c->fd：连接的文件描述符
vec->iovs：iovec数组指针（包含多个缓冲区地址和长度）
vec->count：iovec数组中的元素个数
writev 系统调用可以一次写入多个不连续的缓冲区数据
*/
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "n = writev(c->fd, vec->iovs, vec->count);\n\n");
	
    n = writev(c->fd, vec->iovs, vec->count);

	// 检查 writev 系统调用是否失败（返回 -1 表示失败）
    if (n == -1) 
    {
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "if (n == -1)\n\n");
    	
        switch (errno) 
        {
			case EAGAIN:
        	
        		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        		dprintf(trace_file_fd, "writev() not ready\n\n");
        		
            	return NIGHT_AGAIN;

        	case EINTR:
        	
        		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        		dprintf(trace_file_fd, "writev() was interrupted\n\n");
        	
            	goto eintr;

        	default:
        		
        		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        		dprintf(trace_file_fd, "writev() failed\n\n");
        		
            	c->write->error = 1;
            	
            	return NIGHT_ERROR;
        }
    }

    return n;
}
