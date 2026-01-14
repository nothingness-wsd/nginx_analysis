#ifndef _NIGHT_HTTP_CONNECTION_H_
#define _NIGHT_HTTP_CONNECTION_H_

struct night_http_connection_s 
{
	night_http_core_srv_conf_t *server;
	//night_chain_t *busy;
};

void
night_http_init_connection(night_connection_t *c);

void
night_http_close_connection(night_connection_t *c);

#endif /* _NIGHT_HTTP_CONNECTION_H_ */
