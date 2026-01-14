#include "night_core.h"
#include "night_http_static_module.h"
#include "night_module.h"
#include "night_http_module_ctx.h"
#include "night_pool.h"
#include "night_cycle.h"
#include "night_http_request.h"
#include "night_http_module.h"
#include "night_conf.h"
#include "night_http_core_module.h"
#include "night_open_file_cache.h"
#include "night_string.h"
#include "night_buf.h"
#include "night_file.h"

night_http_module_ctx_t night_http_static_module_ctx =
{
	NULL,                                  		// create main configuration
	NULL,                                  		// create server configuration
	NULL,										// create location configuration
	night_http_static_init						//	postconfiguration
};

night_module_t	night_http_static_module =
{
    "night_http_static_module",					// name;
    NIGHT_MODULE_UNSET_INDEX, 					// index;
    NIGHT_MODULE_UNSET_INDEX, 					// ctx_index;
    NIGHT_HTTP_MODULE, 							// type;
    NULL,										// module directives
    &night_http_static_module_ctx,				// module context
    NULL,										// init_process
    NULL,										// exit_process
    NULL										// exit_master
};

int
night_http_static_init(night_conf_t *cf)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_http_static_init\n\n");
	
    night_http_handler_pt        *h;
    night_http_core_main_conf_t  *cmcf;

    cmcf = ((night_http_conf_t *) cf->ctx)->main_conf[night_http_core_module.ctx_index];

    h = night_array_push(&cmcf->phases[NIGHT_HTTP_CONTENT_PHASE].handlers);
    if (h == NULL) 
    {
        return NIGHT_ERROR;
    }

    *h = night_http_static_handler;
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "*h = night_http_static_handler;\n");
	dprintf(trace_file_fd, "return NIGHT_OK;\n\n");
	
    return NIGHT_OK;
}

/*
这段代码是 Nginx 1.24.0 中处理静态文件请求的核心函数 ngx_http_static_handler 的完整实现。它的作用是：当客户端请求一个 URI 时，如果该 URI 对应的是一个静态文件（而不是目录、动态脚本等），Nginx 会调用此函数来读取该文件并返回给客户端
*/
int
night_http_static_handler(night_http_request_t *r)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_http_static_handler\n\n");
	
	char						*last;
	night_str_t					path;
	size_t						root;
	night_open_file_info_t		of;
	night_http_core_loc_conf_t  *clcf;
	night_buf_t                 *b;
	int							rc;
	night_chain_t				out;
	
	/*
	方法检查：只允许 GET/HEAD/POST
	检查 HTTP 方法是否为 GET、HEAD 或 POST。
	如果不是（如 PUT、DELETE），返回 405 Method Not Allowed。
	*/
    if (!(r->method & (NIGHT_HTTP_GET|NIGHT_HTTP_HEAD|NIGHT_HTTP_POST))) 
    {
    
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "if (!(r->method & (NIGHT_HTTP_GET|NIGHT_HTTP_HEAD|NIGHT_HTTP_POST)))\n");
    	dprintf(trace_file_fd, "return NIGHT_HTTP_NOT_ALLOWED;\n\n");
    	
        return NIGHT_HTTP_NOT_ALLOWED;
    }
    
	/*
	目录请求：拒绝以 / 结尾的 URI
	如果 URI 以 / 结尾（如 /images/），说明客户端请求的是目录
	静态模块不处理目录（应由 ngx_http_autoindex_module 或 index 指令处理）
	返回 NGX_DECLINED 表示“我不处理，请交给下一个 handler”
	*/
    if (r->uri.data[r->uri.len - 1] == '/') 
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "if (r->uri.data[r->uri.len - 1] == '/')\n");
    	dprintf(trace_file_fd, "return NIGHT_DECLINED;\n\n");
    	
        return NIGHT_DECLINED;
    }
    
	/*
	将 URI 映射为文件系统路径
	调用 ngx_http_map_uri_to_path：
	根据 root 或 alias 配置，将请求的 URI（如 /index.html）转换为文件系统路径（如 /var/www/html/index.html）。
	分配内存给 path.data，并返回指向路径末尾的指针 last。
	如果失败（内存不足等），返回 500 Internal Server Error。
	*/
    last = night_http_map_uri_to_path(r, &path, &root, 0);
    if (last == NULL) 
    {
        return NIGHT_HTTP_INTERNAL_SERVER_ERROR;
    }
    
	path.len = last - path.data;
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "path.len=%ld\n", path.len);
	dprintf(trace_file_fd, "path.data=%s\n\n", path.data);
    
	// 获取核心 location 配置
    clcf = r->loc_conf[night_http_core_module.ctx_index];
    
	// 初始化文件缓存结构体
    night_memzero(&of, sizeof(night_open_file_info_t));

/*
作用：
设置文件元信息在 open_file_cache 中的有效时间（秒）。
背景知识：
Nginx 的 open_file_cache 指令可以缓存文件的描述符、大小、mtime 等信息，避免频繁 stat()/open()。
open_file_cache_valid 指令定义缓存条目多久后需要重新验证（默认 60 秒）。
意义：
of.valid 告诉缓存机制：“这个文件的信息在 valid 秒内是可信的”。
超过该时间后，即使缓存命中，也会重新检查文件是否被修改或删除。
*/
    of.valid = 60;
    of.min_uses = 1;

/*
尝试从缓存或磁盘打开文件
调用 ngx_open_cached_file：
先查 open_file_cache（如果启用）。
若未命中，则实际 open() 文件，并缓存元数据（fd、size、mtime 等）。
如果打开失败，进入错误处理。
*/
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "night_open_cached_file\n\n");
	
    if (night_open_cached_file(clcf->open_file_cache, &path, &of, r->pool)
        != NIGHT_OK)
    {
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "night_open_cached_file() != NIGHT_OK\n\n");
		
	}
    
	// 禁止 POST 方法
    if (r->method == NIGHT_HTTP_POST) 
    {
        return NIGHT_HTTP_NOT_ALLOWED;
    }
    
	// 设置响应头
	//状态码：200 OK。
	//Content-Length：文件大小。
	//Last-Modified：文件修改时间。
    r->headers_out.status = NIGHT_HTTP_OK;
    r->headers_out.content_length_n = of.size;
    r->headers_out.last_modified_time = of.mtime;
    
	// 设置 ETag 和 Content-Type
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "night_http_set_etag\n\n");
	
    if (night_http_set_etag(r) != NIGHT_OK) 
    {
        return NIGHT_HTTP_INTERNAL_SERVER_ERROR;
    }

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "night_http_set_content_type\n\n");
	
	
    if (night_http_set_content_type(r) != NIGHT_OK)
    {
        return NIGHT_HTTP_INTERNAL_SERVER_ERROR;
    }
    
	// 分配一个 ngx_buf_t 缓冲区，并为其 file 字段分配内存。
	//此缓冲区将用于发送文件内容
	b = night_pmalloc(r->pool, sizeof(night_buf_t));
    if (b == NULL) 
    {
        return NIGHT_HTTP_INTERNAL_SERVER_ERROR;
    }
    
	b->file = night_pmalloc(r->pool, sizeof(night_file_t));
    if (b->file == NULL) 
    {
        return NIGHT_HTTP_INTERNAL_SERVER_ERROR;
    }
    
	// 发送 HTTP 响应头（状态行 + headers）。
	// 如果是 HEAD 请求（header_only == 1），则无需发送 body，直接返回。
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "rc = night_http_send_header(r);\n\n");
	
    rc = night_http_send_header(r);
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "%d = night_http_send_header(r);\n\n", rc);

	if (rc == NIGHT_ERROR || rc > NIGHT_OK || r->header_only) 
	{
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "return rc(%d);\n\n", rc);
    	
        return rc;
    }
    
	// 初始化文件缓冲区
	// 设置从文件的起始位置（字节偏移 0）开始读取
    b->file_pos = 0;
    
    // 设置文件的结束位置（即文件总大小）
    b->file_last = of.size;

	// 标记缓冲区类型：是否来自文件
    b->in_file = b->file_last ? 1 : 0;
    
    // 标记是否是主请求的最后一个缓冲区
    b->last_buf = (r == r->main) ? 1 : 0;
    
    // 标记是否是当前 buffer 链的最后一个
    b->last_in_chain = 1;
    
    // 设置 sync 标志，表示该 buffer 是否为“同步控制点”
    b->sync = (b->last_buf || b->in_file) ? 0 : 1;

	// 关联文件描述符（fd）
    b->file->fd = of.fd;
    
    //关联文件路径
    b->file->filename = path;

	out.buf = b;
    out.next = NULL;
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "return night_http_output_filter(r, &out);\n\n");
    
	// 发送响应体
    return night_http_output_filter(r, &out);
}
