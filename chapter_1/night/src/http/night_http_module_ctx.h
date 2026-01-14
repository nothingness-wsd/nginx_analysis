#ifndef _NIGHT_HTTP_MODULE_CTX_H_
#define _NIGHT_HTTP_MODULE_CTX_H_

typedef struct night_http_module_ctx_s night_http_module_ctx_t;

struct night_http_module_ctx_s
{
    int   	(*preconfiguration)(night_conf_t *cf);
    int   	(*postconfiguration)(night_conf_t *cf);

    void	*(*create_main_conf)(night_conf_t *cf);
    char	*(*init_main_conf)(night_conf_t *cf, void *conf);

    void	*(*create_srv_conf)(night_conf_t *cf);
    char	*(*merge_srv_conf)(night_conf_t *cf, void *prev, void *conf);

    void	*(*create_loc_conf)(night_conf_t *cf);
    char	*(*merge_loc_conf)(night_conf_t *cf, void *prev, void *conf);
};

#endif /* _NIGHT_HTTP_MODULE_CTX_H_ */
