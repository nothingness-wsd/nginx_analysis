#include "night_core.h"

static int	night_event_max_module;

static char *
night_event_init_conf(night_cycle_t *cycle, void *conf)
{

}

static char *
night_events_block(night_conf_t *cf, night_command_t *cmd, void *conf)
{
    char						*rv;
    void						***ctx;
    int            				i;
    night_conf_t            	pcf;
    night_module_t				*m;
    night_event_module_ctx_t	*mc;

	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "%s\n\n", __func__);
    
    if (*(void **) conf) 
    {
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "function %s:\t" "return \"is duplicate\"\n\n", __func__);
    
        return "is duplicate";
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "统计 event 类型模块的数量并分配索引\n");
	dprintf(trace_file_fd, "night_count_modules\n\n");
	
    night_event_max_module = night_count_modules(cf->cycle, NIGHT_EVENT_MODULE);

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "统计 event 类型模块的数量并分配索引 完成\n");
	dprintf(trace_file_fd, "night_count_modules completed\n\n");
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "为 event 模块配置结构体指针分配内存\n\n");
	
	ctx = night_pcalloc(cf->pool, sizeof(void *));
    if (ctx == NULL)
    {
        return NIGHT_CONF_ERROR;
    }

    *ctx = night_pcalloc(cf->pool, night_event_max_module * sizeof(void *));
    if (*ctx == NULL) 
    {
        return NIGHT_CONF_ERROR;
    }

    *(void **) conf = ctx;
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "遍历所有 event 模块,调用模块的 create_conf 函数创建模块的配置结构体\n\n");
	
	for (i = 0; cf->cycle->modules[i]; i++) 
	{
		m = cf->cycle->modules[i];
		
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "module name=%s\n\n", m->name);
		
        if (m->type != NIGHT_EVENT_MODULE) 
        {
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(trace_file_fd, "该模块非 event 模块，跳过\n\n");
			
            continue;
        }

        mc = m->ctx;

        if (mc->create_conf) 
        {
        	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "创建 %s 模块的配置结构体\n\n", m->name);
        	
            (*ctx)[m->ctx_index] = mc->create_conf(cf->cycle);
            if ((*ctx)[m->ctx_index] == NULL) 
            {
                return NIGHT_CONF_ERROR;
            }
        }
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "解析 events 块中的指令\n\n");
    
	pcf = *cf;
    cf->ctx = ctx;
    cf->module_type = NIGHT_EVENT_MODULE;
    cf->cmd_type = NIGHT_EVENT_CONF;

    rv = night_conf_parse(cf, NULL);

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "解析 events 块中的指令 完成\n\n");
	
	*cf = pcf;
	
	if (rv != NIGHT_CONF_OK) 
	{	
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "解析 events 块中的指令 失败\n");
		dprintf(trace_file_fd, "function %s:\t" "return\n\n", __func__);
	
        return rv;
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "调用各事件模块的 init_conf 函数初始化模块配置\n\n");
	
	for (i = 0; cf->cycle->modules[i]; i++) 
	{
		m = cf->cycle->modules[i];
		
        if (m->type != NIGHT_EVENT_MODULE) 
        {
            continue;
        }

        mc = cf->cycle->modules[i]->ctx;

        if (mc->init_conf) 
        {
        	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "初始化 %s 模块的配置\n\n", m->name);
        	
            rv = mc->init_conf(cf->cycle, (*ctx)[m->ctx_index]);
            if (rv != NIGHT_CONF_OK) 
            {
                return rv;
            }
        }
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function %s:\t" "return NIGHT_CONF_OK\n\n", __func__);
	
    return NIGHT_CONF_OK;
    
}

static night_command_t	night_events_commands[] = 
{
    { 
    	night_string("events"),
      	NIGHT_MAIN_CONF | NIGHT_CONF_BLOCK | NIGHT_CONF_NOARGS,
      	night_events_block,
      	0,
      	0,
      	NULL 
	},

	night_null_command
};

static night_core_module_ctx_t	night_events_module_ctx = 
{
    night_string("events"),
    NULL,
    night_event_init_conf
};

night_module_t	night_events_module = 
{
    "night_events_module",					// name
    NIGHT_MODULE_UNSET_INDEX,				// index
    NIGHT_MODULE_UNSET_INDEX,				// ctx_index
    NIGHT_CORE_MODULE,						// type 
    &night_events_module_ctx,				// module context
    night_events_commands					// module directives
};


