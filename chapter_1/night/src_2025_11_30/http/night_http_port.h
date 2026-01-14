#ifndef _NIGHT_HTTP_PORT_H_
#define _NIGHT_HTTP_PORT_H_

struct night_http_port_s
{
    in_port_t 					port;
    int 						family;
    night_http_listen_opt_t 	*listen_opt;
    night_http_core_srv_conf_t 	*server;
};

#endif /* _NIGHT_HTTP_PORT_H_ */
