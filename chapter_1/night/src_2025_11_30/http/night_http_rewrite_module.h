#ifndef _NIGHT_HTTP_REWRITE_MODULE_H_
#define _NIGHT_HTTP_REWRITE_MODULE_H_

typedef struct night_http_rewrite_loc_conf_s night_http_rewrite_loc_conf_t;

struct night_http_rewrite_loc_conf_s
{
    //ngx_array_t  *codes;        

    //ngx_uint_t    stack_size;

    //ngx_flag_t    log;
    //ngx_flag_t    uninitialized_variable_warn;
};

void*
night_http_rewrite_create_loc_conf(night_conf_t *cf);

int
night_http_rewrite_init(night_conf_t *cf);

int
night_http_rewrite_handler(night_http_request_t *r);

#endif /* _NIGHT_HTTP_REWRITE_MODULE_H_ */
