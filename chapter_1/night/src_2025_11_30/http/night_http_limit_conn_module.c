#include "night_core.h"
#include "night_http_limit_conn_module.h"
#include "night_module.h"
#include "night_http_module_ctx.h"
#include "night_pool.h"
#include "night_cycle.h"
#include "night_http_request.h"
#include "night_http_module.h"
#include "night_conf.h"
#include "night_http_core_module.h"

night_command_t night_http_limit_conn_commands[] = 
{
	night_null_command
};

night_http_module_ctx_t night_http_limit_conn_module_ctx = 
{
	NULL,                                  	// create main configuration
	NULL,                                  	// create server configuration
	night_http_limit_conn_create_conf,		// create location configuration
	night_http_limit_conn_init				//	postconfiguration
};

night_module_t  night_http_limit_conn_module = 
{
	"night_http_limit_conn_module",
	NIGHT_MODULE_UNSET_INDEX,
	NIGHT_MODULE_UNSET_INDEX,
	NIGHT_HTTP_MODULE,
	night_http_limit_conn_commands,
    &night_http_limit_conn_module_ctx,
	NULL,
    NULL,
    NULL
};

void*
night_http_limit_conn_create_conf(night_conf_t *cf)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_http_limit_conn_create_conf\n\n");
	
    night_http_limit_conn_conf_t  *conf;

    conf = night_pmalloc(night_cycle->pool, sizeof(night_http_limit_conn_conf_t));
    if (conf == NULL) 
    {
        return NULL;
    }

    return conf;
}

int
night_http_limit_conn_init(night_conf_t *cf)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_http_limit_conn_init\n\n");
	
	night_http_core_main_conf_t  			*cmcf;
	night_http_handler_pt        			*h;
	
	cmcf = ((night_http_conf_t *) cf->ctx)->main_conf[night_http_core_module.ctx_index];

    h = night_array_push(&cmcf->phases[NIGHT_HTTP_PREACCESS_PHASE].handlers);
    if (h == NULL) 
    {
        return NIGHT_ERROR;
    }

    *h = night_http_limit_conn_handler;
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "*h = night_http_limit_conn_handler;\n");
	dprintf(trace_file_fd, "return NIGHT_OK;\n\n");
	
    return NIGHT_OK;
}

int
night_http_limit_conn_handler(night_http_request_t *r)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_http_limit_conn_handler\n\n");
	
	if (r->main->limit_conn_status) 
	{
    	
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" "if (r->main->limit_conn_status)\nreturn NIGHT_DECLINED;\n\n" , __FILE__, __LINE__);
    	
        return NIGHT_DECLINED;
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" "return NIGHT_DECLINED;\n\n" , __FILE__, __LINE__);
	
    return NIGHT_DECLINED;
}
