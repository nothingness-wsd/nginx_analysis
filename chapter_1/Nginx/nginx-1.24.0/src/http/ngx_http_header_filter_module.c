
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <nginx.h>


static ngx_int_t ngx_http_header_filter_init(ngx_conf_t *cf);
static ngx_int_t ngx_http_header_filter(ngx_http_request_t *r);


static ngx_http_module_t  ngx_http_header_filter_module_ctx = {
    NULL,                                  /* preconfiguration */
    ngx_http_header_filter_init,           /* postconfiguration */

    NULL,                                  /* create main configuration */
    NULL,                                  /* init main configuration */

    NULL,                                  /* create server configuration */
    NULL,                                  /* merge server configuration */

    NULL,                                  /* create location configuration */
    NULL,                                  /* merge location configuration */
};


ngx_module_t  ngx_http_header_filter_module = {
    NGX_MODULE_V1,
    &ngx_http_header_filter_module_ctx,    /* module context */
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


static u_char ngx_http_server_string[] = "Server: nginx" CRLF;
static u_char ngx_http_server_full_string[] = "Server: " NGINX_VER CRLF;
static u_char ngx_http_server_build_string[] = "Server: " NGINX_VER_BUILD CRLF;


static ngx_str_t ngx_http_status_lines[] = {

    ngx_string("200 OK"),
    ngx_string("201 Created"),
    ngx_string("202 Accepted"),
    ngx_null_string,  /* "203 Non-Authoritative Information" */
    ngx_string("204 No Content"),
    ngx_null_string,  /* "205 Reset Content" */
    ngx_string("206 Partial Content"),

    /* ngx_null_string, */  /* "207 Multi-Status" */

#define NGX_HTTP_LAST_2XX  207
#define NGX_HTTP_OFF_3XX   (NGX_HTTP_LAST_2XX - 200)

    /* ngx_null_string, */  /* "300 Multiple Choices" */

    ngx_string("301 Moved Permanently"),
    ngx_string("302 Moved Temporarily"),
    ngx_string("303 See Other"),
    ngx_string("304 Not Modified"),
    ngx_null_string,  /* "305 Use Proxy" */
    ngx_null_string,  /* "306 unused" */
    ngx_string("307 Temporary Redirect"),
    ngx_string("308 Permanent Redirect"),

#define NGX_HTTP_LAST_3XX  309
#define NGX_HTTP_OFF_4XX   (NGX_HTTP_LAST_3XX - 301 + NGX_HTTP_OFF_3XX)

    ngx_string("400 Bad Request"),
    ngx_string("401 Unauthorized"),
    ngx_string("402 Payment Required"),
    ngx_string("403 Forbidden"),
    ngx_string("404 Not Found"),
    ngx_string("405 Not Allowed"),
    ngx_string("406 Not Acceptable"),
    ngx_null_string,  /* "407 Proxy Authentication Required" */
    ngx_string("408 Request Time-out"),
    ngx_string("409 Conflict"),
    ngx_string("410 Gone"),
    ngx_string("411 Length Required"),
    ngx_string("412 Precondition Failed"),
    ngx_string("413 Request Entity Too Large"),
    ngx_string("414 Request-URI Too Large"),
    ngx_string("415 Unsupported Media Type"),
    ngx_string("416 Requested Range Not Satisfiable"),
    ngx_null_string,  /* "417 Expectation Failed" */
    ngx_null_string,  /* "418 unused" */
    ngx_null_string,  /* "419 unused" */
    ngx_null_string,  /* "420 unused" */
    ngx_string("421 Misdirected Request"),
    ngx_null_string,  /* "422 Unprocessable Entity" */
    ngx_null_string,  /* "423 Locked" */
    ngx_null_string,  /* "424 Failed Dependency" */
    ngx_null_string,  /* "425 unused" */
    ngx_null_string,  /* "426 Upgrade Required" */
    ngx_null_string,  /* "427 unused" */
    ngx_null_string,  /* "428 Precondition Required" */
    ngx_string("429 Too Many Requests"),

#define NGX_HTTP_LAST_4XX  430
#define NGX_HTTP_OFF_5XX   (NGX_HTTP_LAST_4XX - 400 + NGX_HTTP_OFF_4XX)

    ngx_string("500 Internal Server Error"),
    ngx_string("501 Not Implemented"),
    ngx_string("502 Bad Gateway"),
    ngx_string("503 Service Temporarily Unavailable"),
    ngx_string("504 Gateway Time-out"),
    ngx_string("505 HTTP Version Not Supported"),
    ngx_null_string,        /* "506 Variant Also Negotiates" */
    ngx_string("507 Insufficient Storage"),

    /* ngx_null_string, */  /* "508 unused" */
    /* ngx_null_string, */  /* "509 unused" */
    /* ngx_null_string, */  /* "510 Not Extended" */

#define NGX_HTTP_LAST_5XX  508

};


ngx_http_header_out_t  ngx_http_headers_out[] = {
    { ngx_string("Server"), offsetof(ngx_http_headers_out_t, server) },
    { ngx_string("Date"), offsetof(ngx_http_headers_out_t, date) },
    { ngx_string("Content-Length"),
                 offsetof(ngx_http_headers_out_t, content_length) },
    { ngx_string("Content-Encoding"),
                 offsetof(ngx_http_headers_out_t, content_encoding) },
    { ngx_string("Location"), offsetof(ngx_http_headers_out_t, location) },
    { ngx_string("Last-Modified"),
                 offsetof(ngx_http_headers_out_t, last_modified) },
    { ngx_string("Accept-Ranges"),
                 offsetof(ngx_http_headers_out_t, accept_ranges) },
    { ngx_string("Expires"), offsetof(ngx_http_headers_out_t, expires) },
    { ngx_string("Cache-Control"),
                 offsetof(ngx_http_headers_out_t, cache_control) },
    { ngx_string("ETag"), offsetof(ngx_http_headers_out_t, etag) },

    { ngx_null_string, 0 }
};

/*
它是 HTTP 响应头生成和发送的核心函数之一，属于 Nginx 的 HTTP 过滤模块链（filter chain） 中的第一个（也是最重要的）过滤器
*/
static ngx_int_t
ngx_http_header_filter(ngx_http_request_t *r)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "ngx_http_header_filter\n\n");
    
    u_char                    *p;
    size_t                     len;
    ngx_str_t                  host, *status_line;
    ngx_buf_t                 *b;
    ngx_uint_t                 status, i, port;
    ngx_chain_t                out;
    ngx_list_part_t           *part;
    ngx_table_elt_t           *header;
    ngx_connection_t          *c;
    ngx_http_core_loc_conf_t  *clcf;
    ngx_http_core_srv_conf_t  *cscf;
    u_char                     addr[NGX_SOCKADDR_STRLEN];

	// 作用：确保响应头只发送一次。
	// 逻辑：如果已发送（header_sent == 1），直接返回；否则标记为已发送。
    if (r->header_sent) {
    	
    	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd,"if (r->header_sent)\nreturn NGX_OK;\n\n");
    	
        return NGX_OK;
    }

    r->header_sent = 1;

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "r->header_sent = 1;\n\n");
	
	
	// 只处理主请求
	// 子请求（subrequest）不生成完整响应头
    if (r != r->main) 
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "if (r != r->main)\nreturn NGX_OK;\n\n");
    	
        return NGX_OK;
    }

	// 仅处理 HTTP/1.0 及以上版本
	// HTTP/0.9 不支持响应头（只有 body），直接跳过
    if (r->http_version < NGX_HTTP_VERSION_10) 
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "if (r->http_version < NGX_HTTP_VERSION_10)\nreturn NGX_OK;\n\n");
    	
        return NGX_OK;
    }
	
	// HEAD 请求只返回头，不返回 body
    if (r->method == NGX_HTTP_HEAD) 
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "if (r->method == NGX_HTTP_HEAD)\nr->header_only = 1;\n\n");
    	
    	// 告诉后续模块不要发送 body
        r->header_only = 1;
    }

/*
Last-Modified 头的合法性检查
Last-Modified 仅对 200、206、304 有效。
若状态码不符合，清除 Last-Modified 相关字段。
在 HTTP 协议中，Last-Modified 响应头字段的使用并不仅限于状态码 200、206 和 304，但它的语义有效性和实际用途确实主要体现在这些状态码中。
Last-Modified 的作用
Last-Modified 是一个响应头，用于指示资源最后一次被修改的日期和时间。它主要用于：
缓存验证（配合 If-Modified-Since 请求头）
条件请求（如 If-Unmodified-Since）
增量同步等场景
哪些状态码下 Last-Modified 是“有意义”的？
通常包含 Last-Modified 的状态码：
200 OK
表示成功返回资源内容，此时提供 Last-Modified 是标准做法，用于后续缓存验证。
206 Partial Content
表示返回了资源的一部分（如断点续传），但仍代表原始资源的一部分，因此可以（也应该）包含原始资源的 Last-Modified。
304 Not Modified
表示客户端缓存仍有效，服务器不返回响应体，但必须在 304 响应中包含与原始 200 响应相同的 Last-Modified（以及其他缓存相关头，如 ETag、Cache-Control 等），以便客户端更新其缓存元数据。
*/
    if (r->headers_out.last_modified_time != -1) 
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "if (r->headers_out.last_modified_time != -1)\n\n");
    	
        if (r->headers_out.status != NGX_HTTP_OK
            && r->headers_out.status != NGX_HTTP_PARTIAL_CONTENT
            && r->headers_out.status != NGX_HTTP_NOT_MODIFIED)
        {
            r->headers_out.last_modified_time = -1;
            r->headers_out.last_modified = NULL;
        }
    }

	// 关闭时禁用 keepalive
	// 当 Nginx 正在关闭（ngx_terminate/ngx_exiting 为真），不再保持连接
    if (r->keepalive && (ngx_terminate || ngx_exiting)) 
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "if (r->keepalive && (ngx_terminate || ngx_exiting))\nr->keepalive = 0;\n\n");
    	
        r->keepalive = 0;
    }

	//  初始化响应头长度
    len = sizeof("HTTP/1.x ") - 1 + sizeof(CRLF) - 1
          /* the end of the header */
          + sizeof(CRLF) - 1;
          
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "len=%ld\n\n", len);  

    /* status line */
	
	// 如果自定义了状态行
    if (r->headers_out.status_line.len) 
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "if (r->headers_out.status_line.len)\n\n");
    	
        len += r->headers_out.status_line.len;
        status_line = &r->headers_out.status_line;
#if (NGX_SUPPRESS_WARN)
        status = 0;
#endif

    } 
    // 否则根据状态码生成
    else {
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "else\n\n");
    	
        status = r->headers_out.status;
        
        dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(trace_file_fd, "status=%ld\n\n", status);
/*
逻辑：
2xx：NGX_HTTP_OK 到 NGX_HTTP_LAST_2XX
特别处理 204 No Content：清空 content-type、content-length 等。
3xx：重定向类
特别处理 304 Not Modified：设 header_only = 1
4xx / 5xx：错误类
状态码映射：
使用 ngx_http_status_lines[] 数组（预定义如 "200 OK"）。
通过偏移量（NGX_HTTP_OFF_3XX 等）索引。
兜底：若状态码未知或字符串为空，则用 ngx_sprintf 动态生成（如 "599 "）。
*/
        if (status >= NGX_HTTP_OK
            && status < NGX_HTTP_LAST_2XX)
        {
        	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "status 2xx\n\n");
        	
        	// 2xx 状态码表示请求已成功被服务器接收、理解并处理。这类状态码属于“成功”类别，意味着客户端的请求达到了预期效果。
            /* 2XX */
			
			// 204 No Content 请求成功，但响应中不包含任何内容
            if (status == NGX_HTTP_NO_CONTENT) {
                r->header_only = 1;
                ngx_str_null(&r->headers_out.content_type);
                r->headers_out.last_modified_time = -1;
                r->headers_out.last_modified = NULL;
                r->headers_out.content_length = NULL;
                r->headers_out.content_length_n = -1;
            }
			
			// 将状态码转换为相对于 200 的偏移量
			// 作为 ngx_http_status_lines[] 数组的索引。
			// 获取对应的状态行字符串
			// 将其长度累加到总响应头长度 len 中，为后续缓冲区分配做准备
            status -= NGX_HTTP_OK;
            status_line = &ngx_http_status_lines[status];
            len += ngx_http_status_lines[status].len;

        } else if (status >= NGX_HTTP_MOVED_PERMANENTLY
                   && status < NGX_HTTP_LAST_3XX)
        {
            /* 3XX */
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "status 3xx\n\n");
        	
            if (status == NGX_HTTP_NOT_MODIFIED) {
                r->header_only = 1;
            }

            status = status - NGX_HTTP_MOVED_PERMANENTLY + NGX_HTTP_OFF_3XX;
            status_line = &ngx_http_status_lines[status];
            len += ngx_http_status_lines[status].len;

        } else if (status >= NGX_HTTP_BAD_REQUEST
                   && status < NGX_HTTP_LAST_4XX)
        {
            /* 4XX */
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "status 4xx\n\n");
        	
            status = status - NGX_HTTP_BAD_REQUEST
                            + NGX_HTTP_OFF_4XX;

            status_line = &ngx_http_status_lines[status];
            len += ngx_http_status_lines[status].len;

        } else if (status >= NGX_HTTP_INTERNAL_SERVER_ERROR
                   && status < NGX_HTTP_LAST_5XX)
        {
            /* 5XX */
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "status 5xx\n\n");
        	
            status = status - NGX_HTTP_INTERNAL_SERVER_ERROR
                            + NGX_HTTP_OFF_5XX;

            status_line = &ngx_http_status_lines[status];
            len += ngx_http_status_lines[status].len;

        } else {
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "other status\n\n");
        	
            len += NGX_INT_T_LEN + 1 /* SP */;
            status_line = NULL;
        }
		
		
        if (status_line && status_line->len == 0) 
        {
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "status_line null\n\n");
        	
            status = r->headers_out.status;
            len += NGX_INT_T_LEN + 1 /* SP */;
            status_line = NULL;
        }
    }
	
	if (status_line)
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "status_line=%s\n\n", status_line->data);
    } 
    
    clcf = ngx_http_get_module_loc_conf(r, ngx_http_core_module);
    
	//  Server 头处理
	// 自动添加 Server: nginx 头（除非用户已设置）
    if (r->headers_out.server == NULL) 
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "if (r->headers_out.server == NULL)\n\n");
    	
        if (clcf->server_tokens == NGX_HTTP_SERVER_TOKENS_ON) {
            len += sizeof(ngx_http_server_full_string) - 1;
            
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            dprintf(trace_file_fd, "len=%ld\n\n", len);

        } else if (clcf->server_tokens == NGX_HTTP_SERVER_TOKENS_BUILD) {
        
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            dprintf(trace_file_fd, "len=%ld\n\n", len);
            
            len += sizeof(ngx_http_server_build_string) - 1;

        } else {
            len += sizeof(ngx_http_server_string) - 1;
            
            dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            dprintf(trace_file_fd, "len=%ld\n\n", len);
        }
    }

	// 添加当前时间（使用 ngx_cached_http_time 缓存）
	// 格式：RFC 1123 格式（如 Date: Sun, 02 Nov 2025 12:00:00 GMT）
    if (r->headers_out.date == NULL) {
        len += sizeof("Date: Mon, 28 Sep 1970 06:00:00 GMT" CRLF) - 1;
        
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "if (r->headers_out.date == NULL)\nlen=%ld\n\n", len);
    }

	// Content-Type 头处理
    if (r->headers_out.content_type.len) 
    {
        len += sizeof("Content-Type: ") - 1
               + r->headers_out.content_type.len + 2;
		
		// 只有当原始 content-type 未包含 charset 时才追加
        if (r->headers_out.content_type_len == r->headers_out.content_type.len
            && r->headers_out.charset.len)
        {
            len += sizeof("; charset=") - 1 + r->headers_out.charset.len;
        }
    }

	// Content-Length 头处理
    if (r->headers_out.content_length == NULL
        && r->headers_out.content_length_n >= 0)
    {
        len += sizeof("Content-Length: ") - 1 + NGX_OFF_T_LEN + 2;
    }

	// Last-Modified 头处理（时间转字符串）
    if (r->headers_out.last_modified == NULL
        && r->headers_out.last_modified_time != -1)
    {
        len += sizeof("Last-Modified: Mon, 28 Sep 1970 06:00:00 GMT" CRLF) - 1;
    }

    c = r->connection;
	
	// Location 重定向处理
	if(r->headers_out.location == NULL)
	{
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "if(r->headers_out.location == NULL)\n\n");
	}
	
    if (r->headers_out.location
        && r->headers_out.location->value.len
        && r->headers_out.location->value.data[0] == '/'
        && clcf->absolute_redirect)
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "r->headers_out.location\n\n");
    	
        r->headers_out.location->hash = 0;

        if (clcf->server_name_in_redirect) {
            cscf = ngx_http_get_module_srv_conf(r, ngx_http_core_module);
            host = cscf->server_name;

        } else if (r->headers_in.server.len) {
            host = r->headers_in.server;

        } else {
            host.len = NGX_SOCKADDR_STRLEN;
            host.data = addr;

            if (ngx_connection_local_sockaddr(c, &host, 0) != NGX_OK) {
                return NGX_ERROR;
            }
        }

        port = ngx_inet_get_port(c->local_sockaddr);

        len += sizeof("Location: https://") - 1
               + host.len
               + r->headers_out.location->value.len + 2;

        if (clcf->port_in_redirect) {

#if (NGX_HTTP_SSL)
            if (c->ssl)
                port = (port == 443) ? 0 : port;
            else
#endif
                port = (port == 80) ? 0 : port;

        } else {
            port = 0;
        }

        if (port) {
            len += sizeof(":65535") - 1;
        }

    } else {
        ngx_str_null(&host);
        port = 0;
    }

	// 启用分块传输编码
    if (r->chunked) 
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "if (r->chunked)\n\n");
    	
        len += sizeof("Transfer-Encoding: chunked" CRLF) - 1;
    }
	
	// Connection 头处理
	// 状态码 101 表示服务器同意客户端的协议升级请求
	// 101 响应必须包含 Connection: Upgrade 头，这是 RFC 6455（WebSocket）和 RFC 7230 的强制要求。
	// 这是协议切换的关键信号
    if (r->headers_out.status == NGX_HTTP_SWITCHING_PROTOCOLS) 
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "if NGX_HTTP_SWITCHING_PROTOCOLS\n\n");
    	
        len += sizeof("Connection: upgrade" CRLF) - 1;

    } 
    else if (r->keepalive) 
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "else if (r->keepalive)\n\n");
    	
        len += sizeof("Connection: keep-alive" CRLF) - 1;

        /*
         * MSIE and Opera ignore the "Keep-Alive: timeout=<N>" header.
         * MSIE keeps the connection alive for about 60-65 seconds.
         * Opera keeps the connection alive very long.
         * Mozilla keeps the connection alive for N plus about 1-10 seconds.
         * Konqueror keeps the connection alive for about N seconds.
         */

        if (clcf->keepalive_header) 
        {
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    		dprintf(trace_file_fd, "if (clcf->keepalive_header)\n\n");
    		
            len += sizeof("Keep-Alive: timeout=") - 1 + NGX_TIME_T_LEN + 2;
        }

    } else 
    {
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "else Connection: close\n\n");
    		
        len += sizeof("Connection: close" CRLF) - 1;
    }

#if (NGX_HTTP_GZIP)
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "#if (NGX_HTTP_GZIP)\n\n");
	
    if (r->gzip_vary) 
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "if (r->gzip_vary)\n\n");
    	
        if (clcf->gzip_vary) {
            len += sizeof("Vary: Accept-Encoding" CRLF) - 1;

        } else {
            r->gzip_vary = 0;
        }
    }
#endif

	// 遍历用户自定义响应头
    part = &r->headers_out.headers.part;
    header = part->elts;

    for (i = 0; /* void */; i++) {

        if (i >= part->nelts) {
            if (part->next == NULL) {
                break;
            }

            part = part->next;
            header = part->elts;
            i = 0;
        }
		
		// 跳过：hash == 0 表示该头被标记为删除
        if (header[i].hash == 0) {
            continue;
        }

        len += header[i].key.len + sizeof(": ") - 1 + header[i].value.len
               + sizeof(CRLF) - 1;
    }

	// 创建缓冲区
    b = ngx_create_temp_buf(r->pool, len);
    if (b == NULL) {
        return NGX_ERROR;
    }

	//写入状态行
    /* "HTTP/1.x " */
    b->last = ngx_cpymem(b->last, "HTTP/1.1 ", sizeof("HTTP/1.x ") - 1);

    /* status line */
    if (status_line) {
        b->last = ngx_copy(b->last, status_line->data, status_line->len);

    } else {
        b->last = ngx_sprintf(b->last, "%03ui ", status);
    }
    *b->last++ = CR; *b->last++ = LF;
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "%s\n\n", b->start);
    
	//  写入 Server 头
    if (r->headers_out.server == NULL) {
        if (clcf->server_tokens == NGX_HTTP_SERVER_TOKENS_ON) {
            p = ngx_http_server_full_string;
            len = sizeof(ngx_http_server_full_string) - 1;

        } else if (clcf->server_tokens == NGX_HTTP_SERVER_TOKENS_BUILD) {
            p = ngx_http_server_build_string;
            len = sizeof(ngx_http_server_build_string) - 1;

        } else {
            p = ngx_http_server_string;
            len = sizeof(ngx_http_server_string) - 1;
        }

        b->last = ngx_cpymem(b->last, p, len);
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "%s\n\n", b->start);

	// 写入 Date 头
    if (r->headers_out.date == NULL) {
        b->last = ngx_cpymem(b->last, "Date: ", sizeof("Date: ") - 1);
        b->last = ngx_cpymem(b->last, ngx_cached_http_time.data,
                             ngx_cached_http_time.len);

        *b->last++ = CR; *b->last++ = LF;
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "%s\n\n", b->start);

	// 写入 Content-Type（含 charset）
    if (r->headers_out.content_type.len) {
        b->last = ngx_cpymem(b->last, "Content-Type: ",
                             sizeof("Content-Type: ") - 1);
        p = b->last;
        b->last = ngx_copy(b->last, r->headers_out.content_type.data,
                           r->headers_out.content_type.len);

        if (r->headers_out.content_type_len == r->headers_out.content_type.len
            && r->headers_out.charset.len)
        {
            b->last = ngx_cpymem(b->last, "; charset=",
                                 sizeof("; charset=") - 1);
            b->last = ngx_copy(b->last, r->headers_out.charset.data,
                               r->headers_out.charset.len);

            /* update r->headers_out.content_type for possible logging */

            r->headers_out.content_type.len = b->last - p;
            r->headers_out.content_type.data = p;
        }

        *b->last++ = CR; *b->last++ = LF;
    }

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "%s\n\n", b->start);
    
    // 写入 Content-Length
    if (r->headers_out.content_length == NULL
        && r->headers_out.content_length_n >= 0)
    {
        b->last = ngx_sprintf(b->last, "Content-Length: %O" CRLF,
                              r->headers_out.content_length_n);
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "%s\n\n", b->start);

	// 写入 Last-Modified
    if (r->headers_out.last_modified == NULL
        && r->headers_out.last_modified_time != -1)
    {
        b->last = ngx_cpymem(b->last, "Last-Modified: ",
                             sizeof("Last-Modified: ") - 1);
        b->last = ngx_http_time(b->last, r->headers_out.last_modified_time);

        *b->last++ = CR; *b->last++ = LF;
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "%s\n\n", b->start);

    if (host.data) {
		
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "host.data=%s\n\n", host.data);
		
        p = b->last + sizeof("Location: ") - 1;

        b->last = ngx_cpymem(b->last, "Location: http",
                             sizeof("Location: http") - 1);

#if (NGX_HTTP_SSL)
        if (c->ssl) {
            *b->last++ ='s';
        }
#endif

        *b->last++ = ':'; *b->last++ = '/'; *b->last++ = '/';
        b->last = ngx_copy(b->last, host.data, host.len);

        if (port) {
            b->last = ngx_sprintf(b->last, ":%ui", port);
        }

        b->last = ngx_copy(b->last, r->headers_out.location->value.data,
                           r->headers_out.location->value.len);

        /* update r->headers_out.location->value for possible logging */

        r->headers_out.location->value.len = b->last - p;
        r->headers_out.location->value.data = p;
        ngx_str_set(&r->headers_out.location->key, "Location");

        *b->last++ = CR; *b->last++ = LF;
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "%s\n\n", b->start);

    if (r->chunked) {
        b->last = ngx_cpymem(b->last, "Transfer-Encoding: chunked" CRLF,
                             sizeof("Transfer-Encoding: chunked" CRLF) - 1);
    }

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "%s\n\n", b->start);
    
    if (r->headers_out.status == NGX_HTTP_SWITCHING_PROTOCOLS) {
        b->last = ngx_cpymem(b->last, "Connection: upgrade" CRLF,
                             sizeof("Connection: upgrade" CRLF) - 1);

    } else if (r->keepalive) {
        b->last = ngx_cpymem(b->last, "Connection: keep-alive" CRLF,
                             sizeof("Connection: keep-alive" CRLF) - 1);

        if (clcf->keepalive_header) {
            b->last = ngx_sprintf(b->last, "Keep-Alive: timeout=%T" CRLF,
                                  clcf->keepalive_header);
        }

    } else {
        b->last = ngx_cpymem(b->last, "Connection: close" CRLF,
                             sizeof("Connection: close" CRLF) - 1);
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "%s\n\n", b->start);

#if (NGX_HTTP_GZIP)
    if (r->gzip_vary) {
        b->last = ngx_cpymem(b->last, "Vary: Accept-Encoding" CRLF,
                             sizeof("Vary: Accept-Encoding" CRLF) - 1);
    }
#endif

	//写入所有自定义响应头
	
    part = &r->headers_out.headers.part;
    header = part->elts;

    for (i = 0; /* void */; i++) {

        if (i >= part->nelts) {
            if (part->next == NULL) {
                break;
            }

            part = part->next;
            header = part->elts;
            i = 0;
        }

        if (header[i].hash == 0) {
            continue;
        }

        b->last = ngx_copy(b->last, header[i].key.data, header[i].key.len);
        *b->last++ = ':'; *b->last++ = ' ';

        b->last = ngx_copy(b->last, header[i].value.data, header[i].value.len);
        *b->last++ = CR; *b->last++ = LF;
    }

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "%s\n\n", b->start);
    
    ngx_log_debug2(NGX_LOG_DEBUG_HTTP, c->log, 0,
                   "%*s", (size_t) (b->last - b->pos), b->pos);

    /* the end of HTTP header */
    *b->last++ = CR; *b->last++ = LF;

    r->header_size = b->last - b->pos;

	// 无 body 的情况
    if (r->header_only) {
    	// last_buf：告诉写过滤器这是最后一个数据块
        b->last_buf = 1;
    }

    out.buf = b;
    out.next = NULL;

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "return ngx_http_write_filter(r, &out);\n\n");
	
	// 调用写过滤器发送数据
    return ngx_http_write_filter(r, &out);
}


static ngx_int_t
ngx_http_header_filter_init(ngx_conf_t *cf)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "ngx_http_header_filter_init\n\n");
    
    ngx_http_top_header_filter = ngx_http_header_filter;

    return NGX_OK;
}
