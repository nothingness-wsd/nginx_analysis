#ifndef _NIGHT_HTTP_MODULE_CTX_H_
#define _NIGHT_HTTP_MODULE_CTX_H_

typedef struct night_http_module_ctx_s night_http_module_ctx_t;

struct night_http_module_ctx_s
{
    void* 	(*create_main_conf)(night_conf_t *cf);
    void* 	(*create_srv_conf)(night_conf_t *cf);
    void* 	(*create_loc_conf)(night_conf_t *cf);
	int		(*postconfiguration)(night_conf_t *cf);
};

#endif /* _NIGHT_HTTP_MODULE_CTX_H_ */
