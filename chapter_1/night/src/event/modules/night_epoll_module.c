#include "night_core.h"

static void *
night_epoll_create_conf(night_cycle_t *cycle)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "%s\n\n", __func__);
	
    night_epoll_conf_t  *epcf;

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "为配置结构分配内存\n\n");
	
    epcf = night_palloc(cycle->pool, sizeof(night_epoll_conf_t));
    if (epcf == NULL) 
    {
        return NULL;
    }

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "设置 \"未设置\" 标识\n\n");
	
    //epcf->events = NIGHT_CONF_UNSET;
    //epcf->aio_requests = NIGHT_CONF_UNSET;

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "返回配置结构体指针\n" "function %s:\t" "return\n\n", __func__);
	
    return epcf;
}

static char *
night_epoll_init_conf(night_cycle_t *cycle, void *conf)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "%s\n\n", __func__);
	
    night_epoll_conf_t *epcf = conf;

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "未设置字段 设置默认值\n\n");
	
    //ngx_conf_init_uint_value(epcf->events, 512);
    //ngx_conf_init_uint_value(epcf->aio_requests, 32);

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function %s:\t" "return NIGHT_CONF_OK\n\n", __func__);
	
    return NIGHT_CONF_OK;
}

static night_command_t	night_epoll_commands[] = 
{

	night_null_command
};

static night_event_module_ctx_t  night_epoll_module_ctx = 
{
	night_string("epoll"),
	night_epoll_create_conf,
	night_epoll_init_conf
};

night_module_t  night_epoll_module = 
{
    "night_epoll_module",
    NIGHT_MODULE_UNSET_INDEX,               
    NIGHT_MODULE_UNSET_INDEX,                  
    NIGHT_EVENT_MODULE,
    &night_epoll_module_ctx,                          
    night_epoll_commands
};

