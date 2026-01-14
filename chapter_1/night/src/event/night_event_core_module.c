#include "night_core.h"

#define DEFAULT_CONNECTIONS	512

static void *
night_event_core_create_conf(night_cycle_t *cycle)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "%s\n\n", __func__);
	
    night_event_conf_t  *ecf;

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "为配置结构体分配内存\n\n");
	
    ecf = night_palloc(cycle->pool, sizeof(night_event_conf_t));
    if (ecf == NULL) 
    {
        return NULL;
    }

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "设置 “未设置” 标识\n\n");
	
    ecf->connections = NIGHT_CONF_UNSET_UINT;

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "返回配置结构体指针\n" "function %s:\t" "return\n\n", __func__);
	
    return ecf;
}


static char *
night_event_core_init_conf(night_cycle_t *cycle, void *conf)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "%s\n\n", __func__);
	
    night_event_conf_t  *ecf = conf;

    int                  		fd;
    int            				i;
    night_module_t        		*module;
    night_event_module_ctx_t	*event_module_ctx;

    module = NULL;

    fd = epoll_create(100);

    if (fd != -1) 
    {
        (void) close(fd);
        module = &night_epoll_module;

    } 
    else if (errno != ENOSYS) 
    {
        module = &night_epoll_module;
    }

    if (module == NULL) 
    {
        dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(trace_file_fd, "no events module found\n");
        dprintf(trace_file_fd, "function %s:\t" "return NIGHT_CONF_ERROR\n\n", __func__);
        
        return NIGHT_CONF_ERROR;
    }
   
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "初始化  配置结构\n\n"); 

    night_conf_init_value(ecf->connections, DEFAULT_CONNECTIONS);
    cycle->connection_n = ecf->connections;

    night_conf_init_value(ecf->use, module->ctx_index);

    event_module_ctx = module->ctx;
    night_conf_init_ptr_value(ecf->name, event_module_ctx->name.data);

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function %s:\t" "return NIGHT_CONF_OK\n\n", __func__);
	
    return NIGHT_CONF_OK;
}

static char *
night_event_connections(night_conf_t *cf, night_command_t *cmd, void *conf)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "function:\t" "%s\n\n", __func__);
    
    night_event_conf_t  *ecf = conf;

    night_str_t  *value;

    if (ecf->connections != NIGHT_CONF_UNSET) 
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "function %s:\t" "return \"is duplicate\"" "\n\n", __func__);
    	
        return "is duplicate";
    }

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "解析并转换指令参数\n\n");
	
    value = cf->args->elts;
    ecf->connections = night_atoi(value[1].data, value[1].len);
    if (ecf->connections == NIGHT_ERROR) 
    {
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "转换指令参数 失败\n"
								"function %s:\t" "return NIGHT_CONF_ERROR\n\n", __func__);
	
        return NIGHT_CONF_ERROR;
    }

    cf->cycle->connection_n = ecf->connections;
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "connection_n=%ld\n\n", cf->cycle->connection_n);
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "function %s:\t" "return NIGHT_CONF_OK\n\n", __func__);

    return NIGHT_CONF_OK;
}


static night_command_t	night_event_core_commands[] = 
{
    { 
    	night_string("worker_connections"),
      	NIGHT_EVENT_CONF | NIGHT_CONF_TAKE1,
      	night_event_connections,
      	0,
      	0,
      	NULL
	},

	night_null_command
};


static night_event_module_ctx_t  night_event_core_module_ctx = 
{
    night_string("event_core"),
    night_event_core_create_conf,
    night_event_core_init_conf
};


night_module_t night_event_core_module = 
{
    "night_event_core_module",				// name
	NIGHT_MODULE_UNSET_INDEX,				// index
    NIGHT_MODULE_UNSET_INDEX,				// ctx_index
    NIGHT_EVENT_MODULE,						// type 
	&night_event_core_module_ctx,			// module context
    night_event_core_commands				// module directives
};



