#ifndef _NIGHT_MODULE_H_
#define _NIGHT_MODULE_H_

/* "CORE" */
#define NIGHT_CORE_MODULE 		(0x45524F43)  

/* "EVNT" */
#define NIGHT_EVENT_MODULE 		(0x544E5645)  

/* "HTTP" */
#define NIGHT_HTTP_MODULE 		(0x50545448) 

#define NIGHT_DIRECT_CONF 		(0x00000001)
#define NIGHT_MAIN_CONF 		(0x00000002)
#define NIGHT_EVENT_CONF 		(0x00000004)
#define NIGHT_HTTP_MAIN_CONF 	(0x00000008)
#define NIGHT_HTTP_SRV_CONF 	(0x00000010)
#define NIGHT_HTTP_LOC_CONF 	(0x00000020)

#define NIGHT_MODULE_UNSET_INDEX (-1)
#define NIGHT_CONF_INT_UNSET	 (-1)

#define night_null_command {night_null_string,0,0,NULL}

#define night_get_conf(conf_ctx, module) conf_ctx[module.index]

#define night_get_event_conf(conf_ctx, module) \
    		(*(void***)(conf_ctx[night_events_module.index]))[module.ctx_index]

struct night_command_s
{
    night_str_t name;
    uint32_t 	type;
    uintptr_t 	offset;
    int 		(*set)(night_conf_t* cf, night_command_t* cmd, void* conf);
};

struct night_module_s
{
    char 				*name;
    int 				index;
    int 				ctx_index;
    uint32_t 			type;
    night_command_t		*commands;
    void				*ctx;
    int 				(*init_process)();
    void 				(*exit_process)();
    void 				(*exit_master)();
};

#endif /* _NIGHT_MODULE_H_ */
