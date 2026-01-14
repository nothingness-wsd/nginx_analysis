#ifndef _NIGHT_HTTP_CONF_CTX_H_
#define _NIGHT_HTTP_CONF_CTX_H_

typedef struct night_http_conf_ctx_s night_http_conf_ctx_t;

struct night_http_conf_ctx_s
{
	void        **main_conf;
    void        **srv_conf;
    void        **loc_conf;
};

#endif /* _NIGHT_HTTP_CONF_CTX_H_ */


