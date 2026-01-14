#ifndef _NIGHT_EVENT_MODULE_CTX_H_
#define _NIGHT_EVENT_MODULE_CTX_H_

typedef struct night_event_module_ctx_s	night_event_module_ctx_t;

struct night_event_module_ctx_s
{
	night_str_t			name;
    void				*(*create_conf)(night_cycle_t *cycle);
    char				*(*init_conf)(night_cycle_t *cycle, void *conf);
	//ngx_event_actions_t     actions;
};

#endif /* _NIGHT_EVENT_MODULE_CTX_H_ */
