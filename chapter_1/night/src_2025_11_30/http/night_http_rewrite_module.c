#include "night_core.h"
#include "night_http_rewrite_module.h"
#include "night_module.h"
#include "night_http_module_ctx.h"
#include "night_pool.h"
#include "night_cycle.h"
#include "night_http_request.h"
#include "night_http_module.h"
#include "night_conf.h"
#include "night_http_core_module.h"

night_command_t  night_http_rewrite_commands[] = 
{
	night_null_command
};

night_http_module_ctx_t  night_http_rewrite_module_ctx = 
{
	NULL,                                  	// create main configuration
	NULL,                                  	// create server configuration
	night_http_rewrite_create_loc_conf,    	// create location configuration
	night_http_rewrite_init					//	postconfiguration
};

night_module_t night_http_rewrite_module = 
{
	"night_http_rewrite_module",
	NIGHT_MODULE_UNSET_INDEX,
	NIGHT_MODULE_UNSET_INDEX,
	NIGHT_HTTP_MODULE,
    night_http_rewrite_commands,
    &night_http_rewrite_module_ctx,
    NULL,
    NULL,
    NULL
};


void*
night_http_rewrite_create_loc_conf(night_conf_t *cf)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_http_rewrite_create_loc_conf\n\n");
	
    night_http_rewrite_loc_conf_t			*loc_conf;

    loc_conf = night_pmalloc(night_cycle->pool, sizeof(night_http_rewrite_loc_conf_t));
    if (loc_conf == NULL) 
    {
		return NULL;
    }

    //conf->stack_size = NGX_CONF_UNSET_UINT;
    //conf->log = NGX_CONF_UNSET;
    //conf->uninitialized_variable_warn = NGX_CONF_UNSET;

    return loc_conf;
}

int
night_http_rewrite_init(night_conf_t *cf)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_http_rewrite_init\n\n");
	
	night_http_core_main_conf_t			*cmcf;
	night_http_handler_pt        		*h;
	
	cmcf = ((night_http_conf_t *) cf->ctx)->main_conf[night_http_core_module.ctx_index];
	
    h = night_array_push(&cmcf->phases[NIGHT_HTTP_SERVER_REWRITE_PHASE].handlers);
    if (h == NULL) 
    {
        return NIGHT_ERROR;
    }
	
    *h = night_http_rewrite_handler;

    h = night_array_push(&cmcf->phases[NIGHT_HTTP_REWRITE_PHASE].handlers);
    if (h == NULL) 
    {
        return NIGHT_ERROR;
    }

    *h = night_http_rewrite_handler;
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "*h = night_http_rewrite_handler;\n");
    dprintf(trace_file_fd, "return NIGHT_OK;\n\n");
    
	return NIGHT_OK;  
}

// ngx_http_rewrite_module 模块的 rewrite 阶段处理器函数：ngx_http_rewrite_handler。
// 它的作用是在 HTTP 请求处理流程的 rewrite 阶段
int
night_http_rewrite_handler(night_http_request_t *r)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_http_rewrite_handler\n\n");

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "return NIGHT_DECLINED;\n\n");
	
	//没有配置任何 rewrite 指令,直接跳过
	return NIGHT_DECLINED;
}

