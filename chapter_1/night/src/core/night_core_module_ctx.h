#ifndef _NIGHT_CORE_MODULE_CTX_
#define _NIGHT_CORE_MODULE_CTX_


typedef struct night_core_module_ctx_s night_core_module_ctx_t;


struct night_core_module_ctx_s
{
    night_str_t			name;
    void               	*(*create_conf)(night_cycle_t *cycle);
    char               	*(*init_conf)(night_cycle_t *cycle, void *conf);
};

#endif /* _NIGHT_CORE_MODULE_CTX_ */
