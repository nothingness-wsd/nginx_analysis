#ifndef	_NIGHT_LISTENING_H_
#define _NIGHT_LISTENING_H_

typedef void (*night_connection_handler_pt)(night_connection_t *c);

struct night_listening_s
{
    struct sockaddr		*sockaddr;
    socklen_t 			socklen;
    night_str_t 		addr_text;
    size_t 				addr_text_max_len;
    void 				*server;
    int 				worker;
    int 				fd;
    int 				type;
    int 				backlog;
    int 				rcvbuf;
    int 				sndbuf;
    
    night_connection_t 	*connection;
    
    // handler of accepted connection
    night_connection_handler_pt handler;
};

int
night_clone_listening(night_listening_t *ls);

int
night_open_listening_sockets();

int
night_configure_listening_sockets();

int
night_close_listening_sockets();

#endif /* _NIGHT_LISTENING_H_ */
