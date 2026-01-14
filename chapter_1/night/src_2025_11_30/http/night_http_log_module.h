#ifndef _NIGHT_HTTP_LOG_MODULE_H_
#define _NIGHT_HTTP_LOG_MODULE_H_

typedef struct night_http_log_main_conf_s 	night_http_log_main_conf_t;
typedef struct night_http_log_loc_conf_s 	night_http_log_loc_conf_t;

struct night_http_log_main_conf_s
{

};

struct night_http_log_loc_conf_s
{

};

void*
night_http_log_create_main_conf(night_conf_t *cf);

void*
night_http_log_create_loc_conf(night_conf_t *cf);

int
night_http_log_init(night_conf_t *cf);

int
night_http_log_handler(night_http_request_t *r);

#endif /* _NIGHT_HTTP_LOG_MODULE_H_ */
