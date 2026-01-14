#ifndef _NIGHT_CORE_MODULE_CTX_H_
#define _NIGHT_CORE_MODULE_CTX_H_

typedef struct night_core_module_ctx_s night_core_module_ctx_t;

struct night_core_module_ctx_s
{
    void* 	(*create_conf)();
    int 	(*init_conf)(void *conf);
};

#endif /* _NIGHT_CORE_MODULE_CTX_H_ */
