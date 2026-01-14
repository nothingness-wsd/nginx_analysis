#include "night_core.h"
#include "night_event_accept.h"
#include "night_socket.h"
#include "night_event.h"
#include "night_connection.h"
#include "night_listening.h"
#include "night_event_timer.h"
#include "night_cycle.h"
#include "night_pool.h"
#include "night_fctl.h"
#include "night_os_io.h"
#include "night_sock.h"

int night_accept_disabled = 0;

/*
它被注册为监听套接字（listening socket）上可读事件（即有新连接到来）的回调函数，
负责接受新连接、初始化连接结构体（ngx_connection_t）、设置事件处理逻辑，并调用该监听端口对应的业务处理函数（ls->handler(c)，通常是 ngx_http_init_connection）
传入一个事件对象 ev，该事件关联的是一个正在监听的 socket（listen socket）
该函数由事件循环（如 epoll、kqueue 等）在监听 socket 可读时调用
*/
void
night_event_accept(night_event_t *ev)
{
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd, "function:\t" "night_event_accept\n\n" );
    
    night_connection_t 					*c, *lc;
    socklen_t 							socklen;
    night_sockaddr_t 					sa;
    night_listening_t 					*ls;
	int 								s;
	night_event_t 						*rev, *wev;

	// 如果该 accept 事件因超时被触发,则重新启用 accept 事件
	// 这通常发生在系统资源不足（如文件描述符耗尽）时，Nginx 主动关闭 accept 事件以避免雪崩，稍后再恢复
	if (ev->timedout) 
	{
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    	dprintf(trace_file_fd, "timedout\n\n");
    	
        if (night_enable_accept_events() != NIGHT_OK) 
        {
            return;
        }

        ev->timedout = 0;
    }
    
    // 用于控制一次 accept 多少连接
    // 默认为 0，即只 accept 一个连接
    ev->available = 0;
    
    // lc 是监听 socket 对应的 night_connection_t
    lc = ev->data;
    // 获取 night_listening_t 配置
    ls = lc->listening;
    // 重置 ready 标志（事件已处理）
    ev->ready = 0;
    
    // 循环，处理连接事件
    do
    {
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    	dprintf(trace_file_fd, "do-while 循环，处理连接事件\n\n");
    	
    	// 初始化 socklen 变量，表示用于接收客户端地址结构的缓冲区大小
    	socklen = sizeof(night_sockaddr_t);
    	
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        dprintf(trace_file_fd, "socklen=%d\n\n", socklen);
        
        // 调用 accept4 接受连接， 同时设置为非阻塞
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
		dprintf(trace_file_fd, "调用 accept4 接受连接， 同时设置为非阻塞\ns = accept4(lc->fd, &sa.sockaddr, &socklen, SOCK_NONBLOCK)\n");
		
		s = accept4(lc->fd, &sa.sockaddr, &socklen, SOCK_NONBLOCK);
		
		dprintf(trace_file_fd, "s=%d\n\n", s);
        
		// 处理 accept 失败    
		if (s == -1) 
		{
			if (errno == EAGAIN) 
			{
				dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
				dprintf(error_log_fd,"accept() not ready\n\n");
				
                return;
            }
            
            dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            dprintf(error_log_fd,"%s accept4() failed\n\n", ls->addr_text.data);
            
			if (errno == ECONNABORTED) 
			{
				if (ev->available) 
				{
                    continue;
                }
			}
			
			if (errno == EMFILE || errno == ENFILE) 
			{
				dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
				dprintf(error_log_fd,"%s accept4() failed\n", ls->addr_text.data);
				dprintf(error_log_fd,"errno=%d:%s\n\n", errno, strerror(errno));
				
				if (night_disable_accept_events(1) != NIGHT_OK)
                {
                    return;
                }
                
                night_event_add_timer(ev, 600);
			}
			
			return;
		}
		
		night_accept_disabled = night_cycle->connection_n / 8 - night_cycle->free_connection_n;
		
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        dprintf(trace_file_fd, "night_accept_disabled = night_cycle->connection_n / 8 - night_cycle->free_connection_n\n");
        dprintf(trace_file_fd, "%d = %d - %d\n\n", night_accept_disabled, night_cycle->connection_n / 8, night_cycle->free_connection_n);  
        
		// 获取新连接结构体
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
		dprintf(trace_file_fd, "获取新连接结构体\nc = night_get_connection(s)\n\n");
		
		c = night_get_connection(s);
        if (c == NULL) 
        {
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        	dprintf(trace_file_fd, "获取新连接结构体 failed\n\n");
        	
            if (close(s) == -1) 
            {
            	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            	dprintf(error_log_fd,"close failed\n\n");
            }

            return;
        }
        
        // 设置连接类型,标记为 TCP 流	
        c->type = SOCK_STREAM;
        
        // 为新连接创建内存池
		c->pool = night_create_pool(night_page_size);
        if (c->pool == NULL) 
        {
            night_close_accepted_connection(c);
            return;
        }
        
        // 保存客户端地址信息
        // 检查系统调用 accept4() 返回的实际地址长度 socklen 是否超过了预分配缓冲区 ngx_sockaddr_t 的最大容量
		if (socklen > (socklen_t) sizeof(night_sockaddr_t)) 
		{
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
			dprintf(trace_file_fd, "accept4() 返回的实际地址长度 socklen 是否超过了预分配缓冲区 night_sockaddr_t 的最大容量\n\n");
			
            socklen = sizeof(night_sockaddr_t);
        }
        
        //分配一块大小为 socklen 的内存，用于存储客户端地址信息
		c->sockaddr = night_pmalloc(c->pool, socklen);
        if (c->sockaddr == NULL) 
        {
            night_close_accepted_connection(c);
            return;
        }
        
        // 将临时地址结构 sa 中的客户端地址数据安全拷贝到连接专属的堆内存 c->sockaddr 中
        memcpy(c->sockaddr, &sa, socklen);
        
        /* 设置 socket 阻塞/非阻塞模式
		if (night_nonblocking(s) == -1) 
		{
			dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(error_log_fd,"night_nonblocking failed\n\n");
			
			night_close_accepted_connection(c);
			return;
		}*/
		
		// 设置读写函数指针，用于后续数据收发
		c->recv = night_io.recv;
        c->send = night_io.send;
     
        c->recv_chain = night_io.recv_chain;
        c->send_chain = night_io.send_chain;

		// 设置地址信息
        c->socklen = socklen;
        c->listening = ls;
        c->local_sockaddr = ls->sockaddr;
        c->local_socklen = ls->socklen;
        
        // 初始化读写事件
		rev = c->read;
        wev = c->write;

        wev->ready = 1;
        rev->ready = 1;
        rev->available = 1;
        
        // 记录连接开始时间
        c->start_time = night_current_msec;
        
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        dprintf(trace_file_fd, "连接开始时间\nc->start_time=%ld\n\n", c->start_time);
        
        // 格式化客户端地址文本
        // 将二进制地址转为字符串
        // 为客户端地址的文本表示分配内存
		c->addr_text.data = night_pmalloc(c->pool, ls->addr_text_max_len);
		if (c->addr_text.data == NULL) 
		{
			night_close_accepted_connection(c);
			return;
		}

		// 将二进制 socket 地址转换为人类可读的字符串，并存入 c->addr_text
		c->addr_text.len = 
		night_sock_ntop(c->sockaddr, c->socklen, c->addr_text.data,ls->addr_text_max_len, 0);
		if (c->addr_text.len == 0) 
		{
			night_close_accepted_connection(c);
			return;
		}
		
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
		dprintf(trace_file_fd, "accepted address is:\n%s\n\n", c->addr_text.data);
		
		/*
		if (night_add_conn(c) == NIGHT_ERROR) 
		{
			dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(error_log_fd,"night_add_conn failed\n\n");
			
			night_close_accepted_connection(c);
			return;
		}
		*/
		
		// 调用业务处理函数
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
		dprintf(trace_file_fd, "调用业务处理函数\nls->handler(c)\n\n");
		
		ls->handler(c);
		
        dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        dprintf(trace_file_fd, "业务处理\nls->handler(c) completed\n\n");

	// 如果 multi_accept on，会继续 accept 直到 EAGAIN 或 available == 0
    } while (ev->available);
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
	dprintf(trace_file_fd, "function night_event_accept:\treturn\n\n");
}

int
night_enable_accept_events()
{
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd, "function:\t" "night_enable_accept_events\n\n" );
    
    int        			i;
    night_listening_t   *ls;
    night_connection_t  *c;

    ls = night_cycle->listening.elts;
    for (i = 0; i < night_cycle->listening.nelts; i++) 
    {
        c = ls[i].connection;

        if (c == NULL || c->read->active) 
        {
            continue;
        }

        if (night_add_event(c->read, NIGHT_READ_EVENT, EPOLLET) == NIGHT_ERROR) 
        {
            return NIGHT_ERROR;
        }
    }

    return NIGHT_OK;
}

int
night_disable_accept_events()
{
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd, "function:\t" "night_disable_accept_events\n\n" );
    
	night_listening_t   *ls;
	int			        i;
	night_connection_t  *c;

    ls = night_cycle->listening.elts;
    for (i = 0; i < night_cycle->listening.nelts; i++) 
    {
        c = ls[i].connection;

        if (c == NULL || !c->read->active) {
            continue;
        }

        if (night_del_event(c->read, NIGHT_READ_EVENT, NIGHT_CLOSE_EVENT) == NIGHT_ERROR)
        {
            return NIGHT_ERROR;
        }
    }

    return NIGHT_OK;
}

void
night_close_accepted_connection(night_connection_t *c)
{
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd, "function:\t" "night_close_accepted_connection\n\n" );

    int  fd;

    night_free_connection(c);

    fd = c->fd;
    c->fd = -1;

    if (close(fd) == -1) 
    {
		dprintf(error_log_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
		dprintf(error_log_fd, "close(%d) failed\n\n", fd);
    }

    if (c->pool) 
    {
        night_destroy_pool(c->pool);
    }
}
