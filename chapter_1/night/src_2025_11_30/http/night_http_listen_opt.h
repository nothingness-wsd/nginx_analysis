#ifndef _NIGHT_HTTP_LISTEN_OPT_H_
#define _NIGHT_HTTP_LISTEN_OPT_H_

struct night_http_listen_opt_s
{
    int 				backlog;
    struct sockaddr		*sockaddr;
    socklen_t 			socklen;
};

#endif /* _NIGHT_HTTP_LISTEN_OPT_H_ */
