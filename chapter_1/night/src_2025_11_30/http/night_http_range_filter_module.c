#include "night_core.h"
#include "night_http_range_filter_module.h"
#include "night_module.h"
#include "night_http_module.h"
#include "night_http_module_ctx.h"
#include "night_http_request.h"
#include "night_conf.h"
#include "night_hash.h"
#include "night_string.h"

/*
ngx_http_range_filter_module 是 Nginx 内置的一个核心模块，其主要作用是支持 HTTP Range 请求，即实现断点续传和多段（multipart）字节范围请求的功能。
*/

night_http_module_ctx_t  night_http_range_header_filter_module_ctx = 
{
	NULL,                                  		// create main configuration
	NULL,                                  		// create server configuration
	NULL,										// create location configuration
	night_http_range_header_filter_init			//	postconfiguration
};

night_module_t	night_http_range_header_filter_module =
{
    "night_http_range_header_filter_module",	// name;
    NIGHT_MODULE_UNSET_INDEX, 					// index;
    NIGHT_MODULE_UNSET_INDEX, 					// ctx_index;
    NIGHT_HTTP_MODULE, 							// type;
    NULL,										// module directives
    &night_http_range_header_filter_module_ctx,	// module context
    NULL,										// init_process
    NULL,										// exit_process
    NULL										// exit_master
};

night_http_module_ctx_t night_http_range_body_filter_module_ctx =
{
	NULL,                                  		// create main configuration
	NULL,                                  		// create server configuration
	NULL,										// create location configuration
	night_http_range_body_filter_init			//	postconfiguration
};

night_module_t	night_http_range_body_filter_module =
{
    "night_http_range_body_filter_module",		// name;
    NIGHT_MODULE_UNSET_INDEX, 					// index;
    NIGHT_MODULE_UNSET_INDEX, 					// ctx_index;
    NIGHT_HTTP_MODULE, 							// type;
    NULL,										// module directives
    &night_http_range_body_filter_module_ctx,	// module context
    NULL,										// init_process
    NULL,										// exit_process
    NULL										// exit_master
};

static night_http_output_header_filter_pt  night_http_next_header_filter;
static night_http_output_body_filter_pt    night_http_next_body_filter;

int
night_http_range_header_filter_init(night_conf_t *cf)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_http_range_header_filter_init\n\n");
    
    night_http_next_header_filter = night_http_top_header_filter;
    night_http_top_header_filter = night_http_range_header_filter;

    return NIGHT_OK;
}


int
night_http_range_body_filter_init(night_conf_t *cf)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_http_range_body_filter_init\n\n");
    
    night_http_next_body_filter = night_http_top_body_filter;
    night_http_top_body_filter = night_http_range_body_filter;

    return NIGHT_OK;
}

static int
night_http_range_header_filter(night_http_request_t *r)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_http_range_header_filter\n\n");
	
/*
这一大条件判断是否跳过 Range 处理，直接调用下一个 header filter。

逐项解释：

r->http_version < NGX_HTTP_VERSION_10
→ 仅 HTTP/1.0 及以上支持 Range。HTTP/0.9 不支持。
r->headers_out.status != NGX_HTTP_OK
→ 只有原始响应是 200 OK 时才可能支持 Range。如果是 404、500 等，不能返回部分内容。
(r != r->main && !r->subrequest_ranges)
→ 如果是子请求（subrequest）且未显式允许子请求使用 Range（subrequest_ranges 为 false），则跳过。
r->headers_out.content_length_n == -1
→ 必须知道完整响应体的长度（Content-Length），否则无法计算 Range。流式响应（如 chunked）不支持 Range。
!r->allow_ranges
→ 模块（如 proxy、fastcgi）可能通过设置 allow_ranges = 0 禁用 Range 支持。
✅ 只要任一条件为真，就跳过 Range 处理，直接进入下一个 header filter。
*/
    if (r->http_version < NIGHT_HTTP_VERSION_10 || 
    	r->headers_out.status != NIGHT_HTTP_OK || 
    	r != r->main || 
    	r->headers_out.content_length_n == -1)
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "return night_http_next_header_filter(r);\n\n");
    	
        return night_http_next_header_filter(r);
    }
    
/* 
检查是否存在有效的 Range 请求头
如果没有 Range 头 → 跳过。
如果 Range 值长度小于 7（"bytes=x" 至少 7 字符）→ 无效。
如果前 6 字符不是 bytes=（不区分大小写）→ 不支持其它单位（如 items），Nginx 只支持 bytes。
✅ 若不符合，跳转到 next_filter：仅添加 Accept-Ranges: bytes 响应头。
*/
    if (r->headers_in.range == NULL || 
    	r->headers_in.range->value.len < 7 || 
    	night_strncasecmp(r->headers_in.range->value.data, (char *) "bytes=", 6) != 0)
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "goto next_filter;\n\n");
    	
        goto next_filter;
    }    

// next_filter：仅添加 Accept-Ranges 响应头  
next_filter:
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "next_filter:\n\n");
	
    r->headers_out.accept_ranges = night_list_push(&r->headers_out.headers);
    if (r->headers_out.accept_ranges == NULL) 
    {
        return NIGHT_ERROR;
    }

    r->headers_out.accept_ranges->hash = 1;
    r->headers_out.accept_ranges->next = NULL;
    night_str_set(&r->headers_out.accept_ranges->key, "Accept-Ranges");
    night_str_set(&r->headers_out.accept_ranges->value, "bytes");

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "return night_http_next_header_filter(r);\n\n");
	
    return night_http_next_header_filter(r);    
}

/*
这是 nginx 的一个 HTTP body filter 模块函数，用于处理客户端请求中包含 Range 头（即部分内容请求）时，对响应体进行截取或重组
*/
int
night_http_range_body_filter(night_http_request_t *r, night_chain_t *in)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_http_range_body_filter\n\n");
	
    night_http_range_filter_ctx_t  *ctx;

	// 检查输入链是否为空
    if (in == NULL) 
    {
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "if (in == NULL)\nreturn night_http_next_body_filter(r, in);\n\n");
		
        return night_http_next_body_filter(r, in);
    }

    ctx = r->ctx[night_http_range_body_filter_module.ctx_index];

	// 如果 ctx 为空，直接透传
    if (ctx == NULL) 
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "if (ctx == NULL)\nreturn night_http_next_body_filter(r, in);\n\n");
    	
        return night_http_next_body_filter(r, in);
    }
}    
