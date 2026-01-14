#include "night_core.h"
#include "night_recv.h"
#include "night_connection.h"
#include "night_event.h"
#include "night_ioctl.h"

ssize_t
night_unix_recv(night_connection_t *c, char *buf, size_t size)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_unix_recv\n\n");
 	
 	night_event_t  	*rev;
 	ssize_t       	n;
 	
 	rev = c->read;
 	
 	// 如果 available == 0 且 没有 pending EOF，说明当前无数据，返回 NGX_AGAIN
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "rev->available=%d\n", rev->available );
	dprintf(trace_file_fd, "rev->pending_eof=%d\n\n", rev->pending_eof);
		
	if (rev->available == 0 && !rev->pending_eof) 
	{
		rev->ready = 0;
		return NIGHT_AGAIN;
	}
    
    do
    {
    	// read data
    	n = recv(c->fd, buf, size, 0);
    	
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "%ld = recv(c->fd, buf, size, 0)\n\n", n);
    	
    	// 读到 0 字节，表示对端已关闭连接（TCP FIN）。
		// 标记事件为非就绪，并设置 EOF。
    	if (n == 0) 
    	{
            rev->ready = 0;
            rev->eof = 1;
            
			return 0;
        } 
        
        if (n > 0)
        {
        	if (rev->available >= 0)
        	{
        		rev->available -= n;
        		
				if (rev->available < 0) 
				{
					rev->available = 0;
					rev->ready = 0;
				}
				
				dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
				dprintf(trace_file_fd, "rev->available=%d\n\n", rev->available);
        	
        	}
        	// 如果 available 未知（初始为 -1），且本次读满了请求的 size 字节，说明可能还有更多数据
        	else if ((size_t) n == size)
        	{
        		// 查询当前 socket 缓冲区中还有多少字节可读，并更新 rev->available
				if (night_socket_nread(c->fd, &rev->available) == -1) 
                {
                	dprintf(error_log_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
                	dprintf(error_log_fd, "night_socket_nread failed\n\n");
                	
                	n = NIGHT_ERROR;

                    break;
                }
                
				dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
				dprintf(trace_file_fd, "rev->available=%d\n\n", rev->available);
        	}
        	
        	// 如果读取字节数 < size，说明已读完当前所有数据（因为 TCP 是流，若还有数据通常会填满 buffer）。
			// 若没有 pending EOF，则标记非就绪。
			// 清空 available。
			// 直接返回 n，不执行后续逻辑。
			if ((size_t) n < size) 
			{
				if (!rev->pending_eof) 
				{
					rev->ready = 0;
				}

				rev->available = 0;
                    
				return n;
			}	
        }
        
		if (errno == EAGAIN || errno == EINTR) 
        {
        	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "recv() not ready\n\n");
        	// 无数据可读，可重试。
            n = NIGHT_AGAIN;

        } 
        else 
        {
        	dprintf(error_log_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(error_log_fd, "recv() failed\n\n");
        	
            break;
        }
            
    }
    // 仅当错误是 EINTR 时重试。其他错误（包括 EAGAIN）不重试，因为 EAGAIN 表示“现在没数据”，重试无意义。
    while (errno == EINTR);
    
    // 无论何种错误或 EAGAIN，都标记事件为“非就绪”，需等待下一次事件通知。
    rev->ready = 0;
    
    // 如果是真实错误（非 NGX_AGAIN），标记事件错误状态
	if (n == NIGHT_ERROR) 
    {
        rev->error = 1;
    }
    
 	return n;
}    

