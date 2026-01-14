#include "night_core.h"
#include "night_connection.h"
#include "night_cycle.h"
#include "night_string.h"
#include "night_event.h"
#include "night_event_timer.h"
#include "night_event_posted.h"

night_connection_t*
night_get_connection(int socket)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_get_connection\n\n");
    
    night_connection_t 						*c;
    night_connection_t 						**files;
    size_t 									nelts;
    size_t 									n;
	night_event_t 							*rev;
	night_event_t 							*wev;
	int 									instance;
    
    if ((uint64_t)socket >= night_cycle->files_n)
    {
    	dprintf(error_log_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd, "the new socket has number %d, but only %lu files are available\n\n",
    	 		socket, night_cycle->files_n);
    	
    	return NULL;
    }
    
    // When the connection pool is nearly full, 
    // try to close some connections 
    // that have not been used for a long time 
    // to free up space for new connections
	night_drain_connections();

    c = night_cycle->free_connections;
    if (c == NULL)
    {
    	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd,"%u worker_connections are not enough\n\n", night_cycle->connection_n);
    	
    	return NULL;
    }
    
	night_cycle->free_connections = c->next;
    night_cycle->free_connection_n--;
    
    if (night_cycle->files[socket] == NULL) 
    {
        night_cycle->files[socket] = c;
    }
    
	rev = c->read;
    wev = c->write;
    
    night_memzero(c, sizeof(night_connection_t));
    
	c->read = rev;
    c->write = wev;
    c->fd = socket;
    
    instance = rev->instance;
    
	night_memzero(rev, sizeof(night_event_t));
    night_memzero(wev, sizeof(night_event_t));
    
	rev->instance = !instance;
    wev->instance = !instance;
    
	rev->index = NIGHT_INVALID_INDEX;
    wev->index = NIGHT_INVALID_INDEX;
    
	rev->data = c;
    wev->data = c;
    
    wev->write = 1;
    
    return c;
}

void
night_free_connection(night_connection_t *c)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_free_connection\n\n");
    
    c->next = night_cycle->free_connections;
    night_cycle->free_connections = c;
    night_cycle->free_connection_n++;

    if (night_cycle->files[c->fd] == c) 
    {
        night_cycle->files[c->fd] = NULL;
    }
}

void 
night_drain_connections()
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_drain_connections\n\n");
    
    night_connection_t	*c;
    int					n;
    int					i;
    night_queue_t		*q;
    
    if(night_cycle->free_connection_n > night_cycle->connection_n / 16
    	|| night_cycle->reusable_connections_n == 0)
    {
    	return;
    }
    
	c = NULL;
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "reusable_connections_n=%d", night_cycle->reusable_connections_n);
	
	n = night_cycle->reusable_connections_n / 8;
	if (n < 1)
	{
		n = 1;
	}
	else if (n > 8)
	{
		n = 8;
	}
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "n=%d\n\n", n);
	
	for (i = 0; i < n; i++)
	{
		if (night_queue_empty(&night_cycle->reusable_connections_queue))
		{
			break;
		}
		
		q = night_queue_last(&night_cycle->reusable_connections_queue);
        c = night_queue_data(q, night_connection_t, queue); 
        
		c->close = 1;
		
		// clear
        c->read->handler(c->read);
	}
	
	if (night_cycle->free_connection_n == 0 && c && c->reusable) 
	{
		c->close = 1;
        c->read->handler(c->read);
	}
}

void
night_close_connection(night_connection_t *c)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_close_connection\n\n");
    
    int fd;
    
	if (c->fd == -1) 
	{
		dprintf(error_log_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(error_log_fd, "connection already closed\n\n");
		
        return;
    }
    
	if (c->read->timer_set) 
	{
        night_event_del_timer(c->read);
    }

    if (c->write->timer_set) 
    {
        night_event_del_timer(c->write);
    }
    
    night_del_conn(c, NIGHT_CLOSE_EVENT);
    
	if (c->read->posted) 
	{
        night_delete_posted_event(c->read);
    }

    if (c->write->posted) 
    {
        night_delete_posted_event(c->write);
    }
    
	c->read->closed = 1;
    c->write->closed = 1;
    
    // 取消连接的“可重用”状态
    night_reusable_connection(c, 0);

    night_free_connection(c);
    
	fd = c->fd;
    c->fd = -1;
    
	if (close(fd) == -1) 
	{
		dprintf(error_log_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(error_log_fd, "close(%d) failed\n\n", fd);
    }
}

void
night_reusable_connection(night_connection_t *c, int reusable)
{
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "function:\t" "night_reusable_connection\n\n");

	// 如果当前连接 已经是可重用状态（c->reusable != 0），则需要先从可重用队列中移除它。
    if (c->reusable) 
    {
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    	dprintf(trace_file_fd, "当前连接 已经是可重用状态,需要先从可重用队列中移除它\n\n");
    	
        night_queue_remove(&c->queue);
        night_cycle->reusable_connections_n--;
    }

	// 关键状态更新：将连接的 reusable 字段设置为传入的新值。
	// 无论之前是否可重用，现在都同步为新的期望状态。
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd, "c->reusable = reusable(%d);\n\n", reusable);
    
    c->reusable = reusable;

	// 如果新状态是 可重用（reusable != 0），则需要将该连接加入可重用队列。
    if (reusable) 
    {
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    	dprintf(trace_file_fd, "将该连接加入可重用队列\n\n");
    	
        night_queue_insert_head((night_queue_t*) &night_cycle->reusable_connections_queue, &c->queue);
        night_cycle->reusable_connections_n++;
        
        dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        dprintf(trace_file_fd, "night_cycle->reusable_connections_n=%d\n\n", night_cycle->reusable_connections_n);
    }
}

int
night_close_idle_connections()
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_close_idle_connections\n\n");
 	
 	int 				i;
 	night_connection_t *c;
 	
 	c = night_cycle->connections;
 	
 	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
 	dprintf(trace_file_fd,"night_cycle->connection_n=%d\n\n", night_cycle->connection_n);
 	
 	for (i = 0; i < night_cycle->connection_n; i++)
 	{
		dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd,"c[%d].fd=%d\n", i, c[i].fd );
		dprintf(trace_file_fd,"c[%d].idle=%d\n\n", i, c[i].idle);
		
 		if (c[i].fd != -1 && c[i].idle)
 		{
			c[i].close = 1;
            c[i].read->handler(c[i].read);
 		}
 	}
 	
 	return NIGHT_OK;   
}

/*
该函数用于在指定的 TCP 连接上启用 TCP_NODELAY 选项，禁用 Nagle 算法，以减少网络延迟，实现数据的立即发送。
*/
int
night_tcp_nodelay(night_connection_t *c)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
	dprintf(trace_file_fd, "function:\t" "night_tcp_nodelay\n\n");
	
	int  	tcp_nodelay;
	int		rc;
	
	tcp_nodelay = 1;
	
/*
调用系统函数 setsockopt 设置 TCP_NODELAY 选项：
c->fd：连接的文件描述符
IPPROTO_TCP：协议级别（TCP 层）
TCP_NODELAY：选项名
(const void *) &tcp_nodelay：选项值的指针
sizeof(int)：选项值的大小
如果设置失败（返回 -1），进入错误处理
*/	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
	dprintf(trace_file_fd, "setsockopt(TCP_NODELAY)\n\n");
	
	rc = setsockopt(c->fd, IPPROTO_TCP, TCP_NODELAY, (void *) &tcp_nodelay, sizeof(int));
	
	if (rc == -1)
	{
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
		dprintf(trace_file_fd, "return NIGHT_ERROR;\n\n");
		
		return NIGHT_ERROR;
	}
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
	dprintf(trace_file_fd, "return NIGHT_OK;\n\n");
	
    return NIGHT_OK;
}	


