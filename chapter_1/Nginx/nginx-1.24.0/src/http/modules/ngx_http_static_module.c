
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


static ngx_int_t ngx_http_static_handler(ngx_http_request_t *r);
static ngx_int_t ngx_http_static_init(ngx_conf_t *cf);


static ngx_http_module_t  ngx_http_static_module_ctx = {
    NULL,                                  /* preconfiguration */
    ngx_http_static_init,                  /* postconfiguration */

    NULL,                                  /* create main configuration */
    NULL,                                  /* init main configuration */

    NULL,                                  /* create server configuration */
    NULL,                                  /* merge server configuration */

    NULL,                                  /* create location configuration */
    NULL                                   /* merge location configuration */
};


ngx_module_t  ngx_http_static_module = {
    NGX_MODULE_V1,
    &ngx_http_static_module_ctx,           /* module context */
    NULL,                                  /* module directives */
    NGX_HTTP_MODULE,                       /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};

/*
这段代码是 Nginx 1.24.0 中处理静态文件请求的核心函数 ngx_http_static_handler 的完整实现。它的作用是：当客户端请求一个 URI 时，如果该 URI 对应的是一个静态文件（而不是目录、动态脚本等），Nginx 会调用此函数来读取该文件并返回给客户端
*/
static ngx_int_t
ngx_http_static_handler(ngx_http_request_t *r)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "ngx_http_static_handler\n\n");
	
    u_char                    *last, *location;
    size_t                     root, len;
    uintptr_t                  escape;
    ngx_str_t                  path;
    ngx_int_t                  rc;
    ngx_uint_t                 level;
    ngx_log_t                 *log;
    ngx_buf_t                 *b;
    ngx_chain_t                out;
    ngx_open_file_info_t       of;
    ngx_http_core_loc_conf_t  *clcf;

	/*
	方法检查：只允许 GET/HEAD/POST
	检查 HTTP 方法是否为 GET、HEAD 或 POST。
	如果不是（如 PUT、DELETE），返回 405 Method Not Allowed。
	*/
    if (!(r->method & (NGX_HTTP_GET|NGX_HTTP_HEAD|NGX_HTTP_POST))) {
    
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "if (!(r->method & (NGX_HTTP_GET|NGX_HTTP_HEAD|NGX_HTTP_POST)))\n");
    	dprintf(trace_file_fd, "return NGX_HTTP_NOT_ALLOWED;\n\n");
    	
        return NGX_HTTP_NOT_ALLOWED;
    }

	/*
	目录请求：拒绝以 / 结尾的 URI
	如果 URI 以 / 结尾（如 /images/），说明客户端请求的是目录
	静态模块不处理目录（应由 ngx_http_autoindex_module 或 index 指令处理）
	返回 NGX_DECLINED 表示“我不处理，请交给下一个 handler”
	*/
    if (r->uri.data[r->uri.len - 1] == '/') {
    	
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "if (r->uri.data[r->uri.len - 1] == '/')\n");
    	dprintf(trace_file_fd, "return NGX_DECLINED;\n\n");
    	
        return NGX_DECLINED;
    }

    log = r->connection->log;

	
    /*
     * ngx_http_map_uri_to_path() allocates memory for terminating '\0'
     * so we do not need to reserve memory for '/' for possible redirect
     */
	/*
	将 URI 映射为文件系统路径
	调用 ngx_http_map_uri_to_path：
	根据 root 或 alias 配置，将请求的 URI（如 /index.html）转换为文件系统路径（如 /var/www/html/index.html）。
	分配内存给 path.data，并返回指向路径末尾的指针 last。
	如果失败（内存不足等），返回 500 Internal Server Error。
	*/
    last = ngx_http_map_uri_to_path(r, &path, &root, 0);
    if (last == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    path.len = last - path.data;
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "path.len=%ld\n", path.len);
	dprintf(trace_file_fd, "path.data=%s\n\n", path.data);
	
	
    ngx_log_debug1(NGX_LOG_DEBUG_HTTP, log, 0,
                   "http filename: \"%s\"", path.data);

	// 获取核心 location 配置
    clcf = ngx_http_get_module_loc_conf(r, ngx_http_core_module);

	// 初始化文件缓存结构体
    ngx_memzero(&of, sizeof(ngx_open_file_info_t));

/*
read_ahead 是 Nginx 的一个指令（read_ahead），用于在 Linux 等系统上设置 POSIX_FADV_SEQUENTIAL 或类似预读提示。
它告诉内核：接下来会顺序读取大量数据，可以提前将后续数据加载到页缓存中，提升大文件读取性能。
默认值通常是 0（不启用），可配置为如 512k。
*/
    of.read_ahead = clcf->read_ahead;
    
/*
设置是否对当前文件使用 Direct I/O（绕过操作系统页缓存）。
背景知识：
directio 是 Nginx 的一个指令（directio），通常用于大文件（如视频）传输。
当文件大小 ≥ directio 设置的阈值（如 directio 512k;）时，Nginx 会使用 Direct I/O（通过 O_DIRECT 标志打开文件）。
Direct I/O 避免了内核页缓存的拷贝，减少内存占用，适用于大文件且不重复访问的场景。
*/    
    of.directio = clcf->directio;
    
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
    of.valid = clcf->open_file_cache_valid;
/*
作用：
设置一个文件在被缓存前所需的最小访问次数。
背景知识：
open_file_cache_min_uses 指令用于防止缓存“一次性”文件（如临时文件）。
例如：open_file_cache_min_uses 2; 表示一个文件必须被访问至少 2 次才会被加入缓存。
意义：
避免缓存污染（cache pollution）。
of.min_uses 会被 ngx_open_cached_file() 用来决定是否值得缓存该文件。    
*/
    of.min_uses = clcf->open_file_cache_min_uses;
/*
作用：
控制是否将文件打开错误（如 404、403）也缓存起来。
背景知识：
open_file_cache_errors on|off; 指令控制是否缓存错误状态。
默认是 off（不缓存错误），设为 on 后，频繁访问不存在的文件不会每次都触发 stat()，而是直接返回缓存的错误。
意义：
提升性能：避免对已知不存在的文件反复进行系统调用。
of.errors = 1 表示允许缓存错误信息。
*/    
    of.errors = clcf->open_file_cache_errors;
    
/*
作用：
控制是否在文件被修改或删除时自动失效缓存（通过 inotify 或 kqueue 等事件通知机制）。
背景知识：
open_file_cache_events on|off;（仅在支持 inotify/kqueue 的系统上有效）。
若启用，Nginx 会监听文件的 IN_DELETE / IN_MOVE 等事件，一旦文件变化，立即从缓存中移除。
意义：
保证缓存的实时性：文件被删除后，下一次请求不会返回旧的缓存内容。
of.events = 1 表示启用事件驱动的缓存失效。
*/    
    of.events = clcf->open_file_cache_events;

    if (ngx_http_set_disable_symlinks(r, clcf, &path, &of) != NGX_OK) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

/*
尝试从缓存或磁盘打开文件
调用 ngx_open_cached_file：
先查 open_file_cache（如果启用）。
若未命中，则实际 open() 文件，并缓存元数据（fd、size、mtime 等）。
如果打开失败，进入错误处理。
*/

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "ngx_open_cached_file\n\n");
	
    if (ngx_open_cached_file(clcf->open_file_cache, &path, &of, r->pool)
        != NGX_OK)
    {
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "ngx_open_cached_file() != NGX_OK\n\n");
		
        switch (of.err) {

        case 0:
            return NGX_HTTP_INTERNAL_SERVER_ERROR;

        case NGX_ENOENT:
        case NGX_ENOTDIR:
        case NGX_ENAMETOOLONG:

            level = NGX_LOG_ERR;
            rc = NGX_HTTP_NOT_FOUND;
            break;

        case NGX_EACCES:
#if (NGX_HAVE_OPENAT)
        case NGX_EMLINK:
        case NGX_ELOOP:
#endif

            level = NGX_LOG_ERR;
            rc = NGX_HTTP_FORBIDDEN;
            break;

        default:

            level = NGX_LOG_CRIT;
            rc = NGX_HTTP_INTERNAL_SERVER_ERROR;
            break;
        }

        if (rc != NGX_HTTP_NOT_FOUND || clcf->log_not_found) {
            ngx_log_error(level, log, of.err,
                          "%s \"%s\" failed", of.failed, path.data);
        }

        return rc;
    }

    r->root_tested = !r->error_page;

    ngx_log_debug1(NGX_LOG_DEBUG_HTTP, log, 0, "http static fd: %d", of.fd);

    if (of.is_dir) {

        ngx_log_debug0(NGX_LOG_DEBUG_HTTP, log, 0, "http dir");

        ngx_http_clear_location(r);

        r->headers_out.location = ngx_list_push(&r->headers_out.headers);
        if (r->headers_out.location == NULL) {
            return NGX_HTTP_INTERNAL_SERVER_ERROR;
        }

        escape = 2 * ngx_escape_uri(NULL, r->uri.data, r->uri.len,
                                    NGX_ESCAPE_URI);

        if (!clcf->alias && r->args.len == 0 && escape == 0) {
            len = r->uri.len + 1;
            location = path.data + root;

            *last = '/';

        } else {
            len = r->uri.len + escape + 1;

            if (r->args.len) {
                len += r->args.len + 1;
            }

            location = ngx_pnalloc(r->pool, len);
            if (location == NULL) {
                ngx_http_clear_location(r);
                return NGX_HTTP_INTERNAL_SERVER_ERROR;
            }

            if (escape) {
                last = (u_char *) ngx_escape_uri(location, r->uri.data,
                                                 r->uri.len, NGX_ESCAPE_URI);

            } else {
                last = ngx_copy(location, r->uri.data, r->uri.len);
            }

            *last = '/';

            if (r->args.len) {
                *++last = '?';
                ngx_memcpy(++last, r->args.data, r->args.len);
            }
        }

        r->headers_out.location->hash = 1;
        r->headers_out.location->next = NULL;
        ngx_str_set(&r->headers_out.location->key, "Location");
        r->headers_out.location->value.len = len;
        r->headers_out.location->value.data = location;

        return NGX_HTTP_MOVED_PERMANENTLY;
    }

#if !(NGX_WIN32) /* the not regular files are probably Unix specific */

    if (!of.is_file) {
        ngx_log_error(NGX_LOG_CRIT, log, 0,
                      "\"%s\" is not a regular file", path.data);

        return NGX_HTTP_NOT_FOUND;
    }

#endif

	// 禁止 POST 方法
    if (r->method == NGX_HTTP_POST) {
        return NGX_HTTP_NOT_ALLOWED;
    }

    rc = ngx_http_discard_request_body(r);

    if (rc != NGX_OK) {
        return rc;
    }

    log->action = "sending response to client";

	// 设置响应头
	//状态码：200 OK。
	//Content-Length：文件大小。
	//Last-Modified：文件修改时间。
    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = of.size;
    r->headers_out.last_modified_time = of.mtime;

	// 设置 ETag 和 Content-Type
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "ngx_http_set_etag\n\n");	
		
    if (ngx_http_set_etag(r) != NGX_OK) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "ngx_http_set_content_type\n\n");
	
    if (ngx_http_set_content_type(r) != NGX_OK) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

	// 支持 HTTP Range 请求（用于断点续传、视频播放等）。
    r->allow_ranges = 1;

	// 分配一个 ngx_buf_t 缓冲区，并为其 file 字段分配内存。
	//此缓冲区将用于发送文件内容
    /* we need to allocate all before the header would be sent */

    b = ngx_calloc_buf(r->pool);
    if (b == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    b->file = ngx_pcalloc(r->pool, sizeof(ngx_file_t));
    if (b->file == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

	// 发送 HTTP 响应头（状态行 + headers）。
	// 如果是 HEAD 请求（header_only == 1），则无需发送 body，直接返回。
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "rc = ngx_http_send_header(r);\n\n");
	
    rc = ngx_http_send_header(r);
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "%ld = ngx_http_send_header(r);\n\n", rc);

    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
    	
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "return rc(%ld);\n\n", rc);
    	
        return rc;
    }

	// 初始化文件缓冲区
	// 设置从文件的起始位置（字节偏移 0）开始读取
    b->file_pos = 0;
    
    // 设置文件的结束位置（即文件总大小）
    b->file_last = of.size;

	// 标记缓冲区类型：是否来自文件
    b->in_file = b->file_last ? 1 : 0;
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "b->in_file=%d\n\n", b->in_file);
    
    // 标记是否是主请求的最后一个缓冲区
    b->last_buf = (r == r->main) ? 1 : 0;
    
    // 标记是否是当前 buffer 链的最后一个
    b->last_in_chain = 1;
    
    // 设置 sync 标志，表示该 buffer 是否为“同步控制点”
    b->sync = (b->last_buf || b->in_file) ? 0 : 1;

	// 关联文件描述符（fd）
    b->file->fd = of.fd;
    
    //关联文件路径
    b->file->name = path;
    b->file->log = log;
    b->file->directio = of.is_directio;

    out.buf = b;
    out.next = NULL;

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "return ngx_http_output_filter(r, &out);\n\n");
    
	// 发送响应体
    return ngx_http_output_filter(r, &out);
}


static ngx_int_t
ngx_http_static_init(ngx_conf_t *cf)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "ngx_http_static_init\n\n");
	
    ngx_http_handler_pt        *h;
    ngx_http_core_main_conf_t  *cmcf;

    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

    h = ngx_array_push(&cmcf->phases[NGX_HTTP_CONTENT_PHASE].handlers);
    if (h == NULL) {
        return NGX_ERROR;
    }

    *h = ngx_http_static_handler;

    return NGX_OK;
}
