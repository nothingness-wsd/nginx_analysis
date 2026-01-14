#ifndef _NIGHT_CONNECTION_H_
#define _NIGHT_CONNECTION_H_

#include "night_queue.h"
#include "night_os_io.h"

struct night_connection_s
{
	int 				fd;
	void 				*data;
	
	void				**ctx;
    void				**main_conf;
    void				**srv_conf;
    void				**loc_conf;
	
	night_listening_t 	*listening;

	night_event_t 		*read;
    night_event_t 		*write;
    
    int 				type;
    
    unsigned 			reusable:1;
	unsigned 			close:1;
	unsigned 			idle:1;
	unsigned 			destroyed:1;
	unsigned			error:1;
	unsigned			timedout:1;
	
	struct sockaddr 	*sockaddr;
	
	night_recv_pt			recv;
    night_send_pt			send;
    night_recv_chain_pt   	recv_chain;
    night_send_chain_pt   	send_chain;
    
	socklen_t 			socklen;
    night_str_t 		addr_text;
	struct sockaddr 	*local_sockaddr;
    socklen_t 			local_socklen;
	
	night_buf_t 		*buffer;
	int64_t 			start_time;
	uint64_t 			requests;
	off_t               sent;
	
    night_connection_t 	*next;
    night_pool_t 		*pool;
	night_queue_t 		queue;
	
};

night_connection_t*
night_get_connection(int socket);

void
night_free_connection(night_connection_t *c);

void 
night_drain_connections();

void
night_close_connection(night_connection_t *c);

void
night_reusable_connection(night_connection_t *c, int reusable);

int
night_close_idle_connections();

int
night_tcp_nodelay(night_connection_t *c);

#endif /* _NIGHT_CONNECTION_H_ */
