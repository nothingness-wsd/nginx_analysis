#include "night_core.h"
#include "night_http_copy_filter_module.h"
#include "night_module.h"
#include "night_http_module.h"
#include "night_http_module_ctx.h"
#include "night_http_request.h"
#include "night_output_chain.h"
#include "night_pool.h"
#include "night_buf.h"


night_http_module_ctx_t night_http_copy_filter_module_ctx =
{
	NULL,                                  		// create main configuration
	NULL,                                  		// create server configuration
	NULL,										// create location configuration
	night_http_copy_filter_init					//	postconfiguration
};

night_module_t	night_http_copy_filter_module =
{
    "night_http_copy_filter_module",			// name;
    NIGHT_MODULE_UNSET_INDEX, 					// index;
    NIGHT_MODULE_UNSET_INDEX, 					// ctx_index;
    NIGHT_HTTP_MODULE, 							// type;
    NULL,										// module directives
    &night_http_copy_filter_module_ctx,			// module context
    NULL,										// init_process
    NULL,										// exit_process
    NULL										// exit_master
};

static night_http_output_body_filter_pt    night_http_next_body_filter;

int
night_http_copy_filter_init(night_conf_t *cf)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_http_copy_filter_init\n\n");
	
    night_http_next_body_filter = night_http_top_body_filter;
    night_http_top_body_filter = night_http_copy_filter;

    return NIGHT_OK;
}

int
night_http_copy_filter(night_http_request_t *r, night_chain_t *in)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_http_copy_filter\n\n");
	
	int								rc;
	night_connection_t             	*c;
	night_output_chain_ctx_t       	*ctx;
	
	c = r->connection;
	
	// 从请求中获取copy filter模块的上下文
	// 如果上下文不存在，则需要创建新的上下文
	ctx = r->ctx[night_http_copy_filter_module.ctx_index];
	
	if (ctx == NULL) 
	{
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "if (ctx == NULL)\n\n");
    	
		ctx = night_pmalloc(r->pool, sizeof(night_output_chain_ctx_t));
        if (ctx == NULL) 
        {
            return NIGHT_ERROR;
        }
        
		// 将新创建的上下文设置到请求中
		r->ctx[night_http_copy_filter_module.ctx_index] = ctx;
		
		ctx->pool = r->pool;
		ctx->bufs.num = 2;
		ctx->bufs.size=32768;
		
		ctx->output_filter = (night_output_chain_filter_pt) night_http_next_body_filter;
		
		ctx->filter_ctx = r;
		
		ctx->tag = &night_http_copy_filter_module;
		
		if (in && in->buf && night_buf_size(in->buf)) 
        {
        	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "r->request_output = 1;\n\n");
        	
            r->request_output = 1;
        }
        
	}
	
	// 执行输出链处理
	// 调用通用输出链函数处理数据
	// 这是核心的数据处理和发送逻辑
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "rc = night_output_chain(ctx, in);\n\n");
	
    rc = night_output_chain(ctx, in);
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "return rc(%d);\n\n", rc);
	
	return rc;
}
