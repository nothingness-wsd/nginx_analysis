#include "night_core.h"
#include "night_http_log_module.h"
#include "night_module.h"
#include "night_http_module.h"
#include "night_http_module_ctx.h"
#include "night_pool.h"
#include "night_cycle.h"
#include "night_conf.h"
#include "night_http_core_module.h"

night_http_module_ctx_t night_http_log_module_ctx =
{
	night_http_log_create_main_conf,			// create main configuration
	NULL,                                  		// create server configuration
	night_http_log_create_loc_conf,				// create location configuration
	night_http_log_init							//	postconfiguration
};

night_module_t	night_http_log_module =
{
    "night_http_log_module",					// name;
    NIGHT_MODULE_UNSET_INDEX, 					// index;
    NIGHT_MODULE_UNSET_INDEX, 					// ctx_index;
    NIGHT_HTTP_MODULE, 							// type;
    NULL,										// module directives
    &night_http_log_module_ctx,					// module context
    NULL,										// init_process
    NULL,										// exit_process
    NULL										// exit_master
};

void*
night_http_log_create_main_conf(night_conf_t *cf)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function:\t" "night_http_log_create_main_conf\n\n");
	
    night_http_log_main_conf_t  *conf;


    conf = night_pmalloc(night_cycle->pool, sizeof(night_http_log_main_conf_t));
    if (conf == NULL) 
    {
        return NULL;
    }

    return conf;
}

void*
night_http_log_create_loc_conf(night_conf_t *cf)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function:\t" "night_http_log_create_loc_conf\n\n");
	
    night_http_log_loc_conf_t  *conf;

    conf = night_pmalloc(night_cycle->pool, sizeof(night_http_log_loc_conf_t));
    if (conf == NULL) 
    {
        return NULL;
    }

    return conf;
}


int
night_http_log_init(night_conf_t *cf)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_http_log_init\n\n");
	
    night_http_handler_pt        *h;
    night_http_core_main_conf_t  *cmcf;

    cmcf = ((night_http_conf_t *) cf->ctx)->main_conf[night_http_core_module.ctx_index];

    h = night_array_push(&cmcf->phases[NIGHT_HTTP_LOG_PHASE].handlers);
    if (h == NULL) 
    {
        return NIGHT_ERROR;
    }

    *h = night_http_log_handler;

    return NIGHT_OK;
}

int
night_http_log_handler(night_http_request_t *r)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_http_log_handler\n\n");
	
	return NIGHT_OK;
}
