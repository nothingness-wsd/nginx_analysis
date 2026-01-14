#include "night_core.h"
#include "night_http_limit_req_module.h"
#include "night_module.h"
#include "night_http_module_ctx.h"
#include "night_pool.h"
#include "night_cycle.h"
#include "night_http_request.h"
#include "night_http_module.h"
#include "night_conf.h"
#include "night_http_core_module.h"

night_command_t night_http_limit_req_commands[] =
{
	night_null_command
};

night_http_module_ctx_t night_http_limit_req_module_ctx =
{
	NULL,                                  	// create main configuration
	NULL,                                  	// create server configuration
	night_http_limit_req_create_conf,		// create location configuration
	night_http_limit_req_init				//	postconfiguration
};

night_module_t	night_http_limit_req_module =
{
    "night_http_limit_req_module",				// name;
    NIGHT_MODULE_UNSET_INDEX, 					// index;
    NIGHT_MODULE_UNSET_INDEX, 					// ctx_index;
    NIGHT_HTTP_MODULE, 							// type;
    night_http_limit_req_commands,				// module directives
    &night_http_limit_req_module_ctx,        	// module context
    NULL,										// init_process
    NULL,										// exit_process
    NULL										// exit_master
};

void*
night_http_limit_req_create_conf(night_conf_t *cf)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_http_limit_req_create_conf\n\n");
	
    night_http_limit_req_conf_t  *conf;

    conf = night_pmalloc(night_cycle->pool, sizeof(night_http_limit_req_conf_t));
    if (conf == NULL) 
    {
        return NULL;
    }

    return conf;
}

int
night_http_limit_req_init(night_conf_t *cf)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_http_limit_req_init\n\n");
	
	night_http_handler_pt        *h;
    night_http_core_main_conf_t  *cmcf;
    
    cmcf = ((night_http_conf_t *) cf->ctx)->main_conf[night_http_core_module.ctx_index];
    
	h = night_array_push(&cmcf->phases[NIGHT_HTTP_PREACCESS_PHASE].handlers);
    if (h == NULL) 
    {
        return NIGHT_ERROR;
    }

    *h = night_http_limit_req_handler;
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "*h = night_http_limit_req_handler;\n");
    dprintf(trace_file_fd, "return NIGHT_OK;\n\n");
    
    return NIGHT_OK;
}

// 它的作用是 实现请求频率限制（Rate Limiting），即根据配置的规则，限制客户端在单位时间内的请求数量，防止服务器过载或被恶意请求攻击
int
night_http_limit_req_handler(night_http_request_t *r)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_http_limit_req_handler\n\n");
	
	int 						rc;
	/*
	如果主请求（r->main）已经设置过 limit_req_status（说明已处理过限流），则直接跳过
	防止子请求重复触发限流逻辑。
	*/
	if (r->main->limit_req_status) 
	{
        return NIGHT_DECLINED;
    }
    
    rc = NIGHT_DECLINED;
    
	if (rc == NIGHT_DECLINED) 
	{
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" "if (rc == NIGHT_DECLINED)\nreturn NIGHT_DECLINED;\n\n" , __FILE__, __LINE__);
        
        return NIGHT_DECLINED;
    }

	return NIGHT_AGAIN;
}
