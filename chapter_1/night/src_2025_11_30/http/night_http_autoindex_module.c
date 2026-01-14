#include "night_core.h"
#include "night_http_autoindex_module.h"
#include "night_module.h"
#include "night_http_module_ctx.h"
#include "night_pool.h"
#include "night_cycle.h"
#include "night_http_request.h"
#include "night_http_module.h"
#include "night_conf.h"
#include "night_http_core_module.h"


night_command_t night_http_autoindex_commands[] =
{
	night_null_command
};

night_http_module_ctx_t night_http_autoindex_module_ctx = 
{
	NULL,									// create main configuration
	NULL,									// create server configuration
	night_http_autoindex_create_loc_conf,	// create location configuration
	night_http_autoindex_init				//	postconfiguration
};

/*
当客户端请求的目录中没有默认索引文件（如 index.html、index.php 等）时，自动生成并返回该目录的文件列表（目录浏览页面）
默认情况下，如果一个请求指向一个目录（例如 http://example.com/files/），而该目录下没有配置的 index 文件（由 index 指令定义），Nginx 会返回 403 Forbidden 错误。
启用 ngx_http_autoindex_module 并配置 autoindex on; 后，Nginx 会自动列出该目录下的文件和子目录，形成一个简单的 HTML 页面供用户浏览和下载。
*/
night_module_t night_http_autoindex_module = 
{
	"night_http_autoindex_module",
	NIGHT_MODULE_UNSET_INDEX,
	NIGHT_MODULE_UNSET_INDEX,
	NIGHT_HTTP_MODULE,
	night_http_autoindex_commands,
	&night_http_autoindex_module_ctx,
	NULL,
	NULL,
	NULL
};

void*
night_http_autoindex_create_loc_conf(night_conf_t *cf)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_http_autoindex_create_loc_conf\n\n");
	
    night_http_autoindex_loc_conf_t  	*conf;

    conf = night_pmalloc(night_cycle->pool, sizeof(night_http_autoindex_loc_conf_t));
    if (conf == NULL) 
    {
        return NULL;
    }

    //conf->enable = NGX_CONF_UNSET;
    //conf->format = NGX_CONF_UNSET_UINT;
    //conf->localtime = NGX_CONF_UNSET;
    //conf->exact_size = NGX_CONF_UNSET;

    return conf;
}


int
night_http_autoindex_init(night_conf_t *cf)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_http_autoindex_init\n\n");
	
    night_http_handler_pt        *h;
    night_http_core_main_conf_t  *cmcf;

	cmcf = ((night_http_conf_t *) cf->ctx)->main_conf[night_http_core_module.ctx_index];

    h = night_array_push(&cmcf->phases[NIGHT_HTTP_CONTENT_PHASE].handlers);
    if (h == NULL) 
    {
        return NIGHT_ERROR;
    }

    *h = night_http_autoindex_handler;

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "*h = night_http_autoindex_handler;\n");
    dprintf(trace_file_fd, "return NIGHT_OK;\n\n");
    
    return NIGHT_OK;
}

/*
ngx_http_autoindex_module 模块的核心函数 ngx_http_autoindex_handler，用于处理目录浏览（即当请求一个目录且该目录下没有默认索引文件时，Nginx 自动列出目录内容）。
*/
int
night_http_autoindex_handler(night_http_request_t *r)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_http_autoindex_handler\n\n");
	
// 检查 URI 是否以 '/' 结尾
//逻辑：只有请求的是目录（URI 以 / 结尾）才处理。
//否则：交给其他模块（如 index 模块尝试找 index.html）
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "r->uri.data=%s\n\n", r->uri.data);
	
    if (r->uri.data[r->uri.len - 1] != '/') 
    {
    	
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "if (r->uri.data[r->uri.len - 1] != '/')\nreturn NIGHT_DECLINED;\n\n");
    	
        return NIGHT_DECLINED;
    }
	return NIGHT_OK;
}
	


