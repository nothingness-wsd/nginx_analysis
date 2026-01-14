#ifndef _NIGHT_HTTP_LIMIT_CONN_MODULE_H_
#define _NIGHT_HTTP_LIMIT_CONN_MODULE_H_

typedef struct night_http_limit_conn_conf_s night_http_limit_conn_conf_t;

struct night_http_limit_conn_conf_s
{

};

void*
night_http_limit_conn_create_conf(night_conf_t *cf);

int
night_http_limit_conn_init(night_conf_t *cf);

int
night_http_limit_conn_handler(night_http_request_t *r);

#endif // _NIGHT_HTTP_LIMIT_CONN_MODULE_H_

