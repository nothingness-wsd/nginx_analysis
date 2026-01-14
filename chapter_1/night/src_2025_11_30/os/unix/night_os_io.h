#ifndef _NIGHT_OS_IO_H_
#define _NIGHT_OS_IO_H_

typedef struct night_os_io_s night_os_io_t;

typedef ssize_t (*night_recv_pt)(night_connection_t *c, char *buf, size_t size);
typedef ssize_t (*night_send_pt)(night_connection_t *c, char *buf, size_t size);

typedef ssize_t (*night_recv_chain_pt)(night_connection_t *c, night_chain_t *in, off_t limit);
typedef night_chain_t *(*night_send_chain_pt)(night_connection_t *c, night_chain_t *in, off_t limit);

struct night_os_io_s 
{
	night_recv_pt        recv;
	night_send_pt        send;
	
	night_recv_chain_pt  recv_chain;
	night_send_chain_pt  send_chain;
};

extern night_os_io_t	night_io;

extern night_os_io_t 	night_os_io;


#endif /* _NIGHT_OS_IO_H_ */



