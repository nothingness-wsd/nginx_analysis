#include "night_core.h"
#include "night_http_postpone_filter_module.h"
#include "night_module.h"
#include "night_http_module.h"
#include "night_http_module_ctx.h"
#include "night_http_request.h"

night_http_module_ctx_t night_http_postpone_filter_module_ctx =
{
	NULL,                                  		// create main configuration
	NULL,                                  		// create server configuration
	NULL,										// create location configuration
	night_http_postpone_filter_init				//	postconfiguration
};

night_module_t	night_http_postpone_filter_module =
{
    "night_http_postpone_filter_module",		// name;
    NIGHT_MODULE_UNSET_INDEX, 					// index;
    NIGHT_MODULE_UNSET_INDEX, 					// ctx_index;
    NIGHT_HTTP_MODULE, 							// type;
    NULL,										// module directives
    &night_http_postpone_filter_module_ctx,		// module context
    NULL,										// init_process
    NULL,										// exit_process
    NULL										// exit_master
};

static night_http_output_body_filter_pt    night_http_next_body_filter;

int
night_http_postpone_filter_init(night_conf_t *cf)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_http_postpone_filter_init\n\n");
	
    night_http_next_body_filter = night_http_top_body_filter;
    night_http_top_body_filter = night_http_postpone_filter;

    return NIGHT_OK;
}

int
night_http_postpone_filter(night_http_request_t *r, night_chain_t *in)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_http_postpone_filter\n\n");
	
	int 						rc;
	night_connection_t			*c;
		
	if (in) 
	{
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "return night_http_next_body_filter(r->main, in);\n\n");
    	
		return night_http_next_body_filter(r->main, in);
	}
	
	return NIGHT_OK;
}	
