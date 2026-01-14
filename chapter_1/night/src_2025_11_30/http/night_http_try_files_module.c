#include "night_core.h"
#include "night_http_try_files_module.h"
#include "night_module.h"
#include "night_http_module_ctx.h"
#include "night_pool.h"
#include "night_cycle.h"
#include "night_http_request.h"
#include "night_http_module.h"
#include "night_conf.h"
#include "night_http_core_module.h"

night_command_t night_http_try_files_commands[] =
{
	night_null_command
};

night_http_module_ctx_t night_http_try_files_module_ctx =
{
	NULL,                                  		// create main configuration
	NULL,                                  		// create server configuration
	night_http_try_files_create_loc_conf,		// create location configuration
	night_http_try_files_init					//	postconfiguration
};

night_module_t	night_http_try_files_module =
{
    "night_http_try_files_module",				// name;
    NIGHT_MODULE_UNSET_INDEX, 					// index;
    NIGHT_MODULE_UNSET_INDEX, 					// ctx_index;
    NIGHT_HTTP_MODULE, 							// type;
    night_http_try_files_commands,				// module directives
    &night_http_try_files_module_ctx,			// module context
    NULL,										// init_process
    NULL,										// exit_process
    NULL										// exit_master
};

void*
night_http_try_files_create_loc_conf(night_conf_t *cf)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_http_try_files_create_loc_conf\n\n");
	
    night_http_try_files_loc_conf_t  *tlcf;

    tlcf = night_pmalloc(night_cycle->pool, sizeof(night_http_try_files_loc_conf_t));
    if (tlcf == NULL) 
    {
        return NULL;
    }

    return tlcf;
}

int
night_http_try_files_init(night_conf_t *cf)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_http_try_files_init\n\n");
	
    night_http_handler_pt        *h;
    night_http_core_main_conf_t  *cmcf;

   	cmcf = ((night_http_conf_t *) cf->ctx)->main_conf[night_http_core_module.ctx_index];

    h = night_array_push(&cmcf->phases[NIGHT_HTTP_PRECONTENT_PHASE].handlers);
    if (h == NULL) 
    {
        return NIGHT_ERROR;
    }

    *h = night_http_try_files_handler;
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "*h = night_http_try_files_handler;\n");
	dprintf(trace_file_fd, "return NIGHT_OK;\n\n");
	
    return NIGHT_OK;
}

// try_files 是 Nginx 中非常常用的指令，用于按顺序检查文件或目录是否存在，如果都不存在则 fallback 到最后一个参数
int
night_http_try_files_handler(night_http_request_t *r)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_http_try_files_handler\n\n");
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" "return NIGHT_DECLINED;\n\n", __FILE__, __LINE__);
	
	return NIGHT_DECLINED;
}
