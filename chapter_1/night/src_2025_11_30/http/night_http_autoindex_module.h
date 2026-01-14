#ifndef _NIGHT_HTTP_AUTOINDEX_MODULE_H_
#define _NIGHT_HTTP_AUTOINDEX_MODULE_H_

typedef struct night_http_autoindex_loc_conf_s night_http_autoindex_loc_conf_t;

struct night_http_autoindex_loc_conf_s
{

};

void*
night_http_autoindex_create_loc_conf(night_conf_t *cf);

int
night_http_autoindex_init(night_conf_t *cf);

int
night_http_autoindex_handler(night_http_request_t *r);

#endif /* _NIGHT_HTTP_AUTOINDEX_MODULE_H_ */
