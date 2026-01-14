#include "night_core.h"
#include "night_linux_sendfile_chain.h"
#include "night_connection.h"
#include "night_event.h"
#include "night_os.h"
#include "night_writev_chain.h"
#include "night_buf.h"

night_chain_t *
night_linux_sendfile_chain(night_connection_t *c, night_chain_t *in, off_t limit)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_linux_sendfile_chain\n\n");
	
	night_event_t		*wev;
	size_t          	send,sent;
	
	night_iovec_t    	header;
    struct iovec   		headers[NIGHT_IOVS_PREALLOCATE];
    size_t				prev_send;
    night_chain_t   	*cl;
    size_t				n;
	
	wev = c->write;

	// 检查写就绪：如果写事件未就绪，直接返回原始链表，不进行发送
    if (!wev->ready) 
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "wev not ready\n\n");
    
        return in;
    }
    
	//限制大小检查：如果limit为0或超过最大sendfile大小（2G减去页面大小），则设置为最大值
	//这是为了避免sendfile系统调用的限制 
    if (limit == 0 || limit > (off_t) (NIGHT_SENDFILE_MAXSIZE - night_page_size)) 
    {
        limit = NIGHT_SENDFILE_MAXSIZE - night_page_size;
    }
	
	// 初始化发送计数：发送字节数初始化为0
    send = 0;
    
	//初始化IO向量：设置IO向量结构，指向预分配的headers数组和分配数量
    header.iovs = headers;
    header.nalloc = NIGHT_IOVS_PREALLOCATE;
    header.count = 0;
    header.size = 0;
    
	// 无限循环：持续处理发送，直到完成或遇到阻塞
	// prev_send记录本次循环开始时的发送字节数
    for ( ;; ) 
    {	
        prev_send = send;
        
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "prev_send=%ld\n\n", prev_send);
    	
    	// 构建IO向量：将链表中的缓冲区转换为iovec数组，合并相邻缓冲区
		// limit - send表示剩余可发送字节数
		
		cl = night_output_chain_to_iovec(&header, in, limit - send);

        if (cl == NIGHT_CHAIN_ERROR) 
        {
            return NIGHT_CHAIN_ERROR;
        }
		
		//更新发送计数：将iovec中数据大小加到总发送量中
        send += header.size;
        
		// 处理文件缓冲区：如果没有header数据、有后续缓冲区、该缓冲区在文件中且未达到限制
        if (header.count == 0 && cl && cl->buf->in_file && send < limit) 
        {
        	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "if (header.count == 0 && cl && cl->buf->in_file && send < limit)\n\n");
        }
        else
        {
        	//发送非文件数据：如果没有文件缓冲区，使用writev发送iovec中的数据
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "n = night_writev(c, &header);\n\n");
        	
            n = night_writev(c, &header);

			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(trace_file_fd, "n=%ld\n\n", n);
			
            if (n == NIGHT_ERROR) 
            {
                return NIGHT_CHAIN_ERROR;
            }

			// 更新连接发送统计：将本次发送字节数加到连接的总发送量中
            sent = (n == NIGHT_AGAIN) ? 0 : n;
            
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            dprintf(trace_file_fd, "sent=%ld\n\n", sent);
        }
        
		// 更新连接发送统计：将本次发送字节数加到连接的总发送量中
        c->sent += sent;
        
        dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(trace_file_fd, "c->sent=%ld\n\n", c->sent);
        
		// 更新链表：根据已发送字节数更新链表指针，移除已发送的数据
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "in = night_chain_update_sent(in, sent);\n\n");
		
        in = night_chain_update_sent(in, sent);
        
        // 如果系统调用返回AGAIN，表示需要等待，设置写事件未就绪并返回
		if (n == NIGHT_AGAIN) 
		{
            wev->ready = 0;
            return in;
        }
        
		// 如果已发送量达到限制或链表为空，返回剩余链表，否则继续循环
        if (send >= limit || in == NULL) 
        {
        	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "if (send >= limit || in == NULL)\nreturn in;\n\n");
        	
            return in;
        }
    }	
}	
