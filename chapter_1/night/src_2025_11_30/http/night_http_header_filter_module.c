#include "night_core.h"
#include "night_http_header_filter_module.h"
#include "night_module.h"
#include "night_http_module.h"
#include "night_http_module_ctx.h"
#include "night_http_request.h"
#include "night_string.h"
#include "night_hash.h"
#include "night_buf.h"
#include "night_time.h"
#include "night_http_write_filter_module.h"

#define NIGHT_HTTP_LAST_2XX  207

night_http_module_ctx_t night_http_header_filter_module_ctx =
{
	NULL,                                  		// create main configuration
	NULL,                                  		// create server configuration
	NULL,										// create location configuration
	night_http_header_filter_init				//	postconfiguration
};

night_module_t	night_http_header_filter_module =
{
    "night_http_header_filter_module",			// name;
    NIGHT_MODULE_UNSET_INDEX, 					// index;
    NIGHT_MODULE_UNSET_INDEX, 					// ctx_index;
    NIGHT_HTTP_MODULE, 							// type;
    NULL,										// module directives
    &night_http_header_filter_module_ctx,		// module context
    NULL,										// init_process
    NULL,										// exit_process
    NULL										// exit_master
};

static night_str_t night_http_status_lines[] = 
{
    night_string("200 OK"),
    night_string("201 Created"),
    night_string("202 Accepted"),
    night_null_string,  /* "203 Non-Authoritative Information" */
    night_string("204 No Content"),
    night_null_string,  /* "205 Reset Content" */
    night_string("206 Partial Content")
};

static char night_http_server_string[] = "Server: night" CRLF;

int
night_http_header_filter_init(night_conf_t *cf)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_http_header_filter_init\n\n");
    
    night_http_top_header_filter = night_http_header_filter;

    return NIGHT_OK;
}

int
night_http_header_filter(night_http_request_t *r)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_http_header_filter\n\n");
    
    size_t						len;
    night_str_t                 *status_line;
    int                 		status;
    night_http_core_loc_conf_t  *clcf;
    night_connection_t          *c;
	night_list_part_t           *part;
    night_table_elt_t           *header;
    size_t						i;
    night_buf_t                 *b;
    char*						p;
    night_chain_t               out;
    
	// 作用：确保响应头只发送一次
	// 逻辑：如果已发送（header_sent == 1），直接返回；否则标记为已发送
    if (r->header_sent) 
    {
		dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd,"if (r->header_sent)\nreturn NIGHT_OK;\n\n");
    	
        return NIGHT_OK;
    }

    r->header_sent = 1;
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "r->header_sent = 1;\n\n");
	
	// 只处理主请求
	// 子请求（subrequest）不生成完整响应头
    if (r != r->main) 
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "if (r != r->main)\nreturn NIGHT_OK;\n\n");
    	
        return NIGHT_OK;
    }
    
	// 仅处理 HTTP/1.0 及以上版本
	// HTTP/0.9 不支持响应头（只有 body），直接跳过
    if (r->http_version < NIGHT_HTTP_VERSION_10) 
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "if (r->http_version < NIGHT_HTTP_VERSION_10)\nreturn NIGHT_OK;\n\n");
    	
        return NIGHT_OK;
    }
    
   	// HEAD 请求只返回头，不返回 body
    if (r->method == NIGHT_HTTP_HEAD) 
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "if (r->method == NIGHT_HTTP_HEAD)\nr->header_only = 1;\n\n");
    	
    	// 告诉后续模块不要发送 body
        r->header_only = 1;
    }
    
    // Last-Modified 仅对 200、206、304 有效
    if (r->headers_out.last_modified_time != -1) 
    {
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "if (r->headers_out.last_modified_time != -1)\n\n");
    	
		if (r->headers_out.status != NIGHT_HTTP_OK &&
			r->headers_out.status != NIGHT_HTTP_PARTIAL_CONTENT && 
			r->headers_out.status != NIGHT_HTTP_NOT_MODIFIED
           )
        {
            r->headers_out.last_modified_time = -1;
            r->headers_out.last_modified = NULL;
        }
    }
	
	// 关闭时禁用 keepalive
    if (r->keepalive && (night_terminate || night_exiting)) 
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "if (r->keepalive && (night_terminate || night_exiting))\nr->keepalive = 0;\n\n");
    	
        r->keepalive = 0;
    }
    
	//  初始化响应头长度
    len = sizeof("HTTP/1.x ") - 1 + sizeof(CRLF) - 1
          /* the end of the header */
          + sizeof(CRLF) - 1;
          
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "len=%ld\n\n", len); 
    
    // 构建状态行
    if (r->headers_out.status_line.len) 
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "if (r->headers_out.status_line.len)\n\n");
    	
		len += r->headers_out.status_line.len;
        status_line = &r->headers_out.status_line;
        status = 0;
    }
    // 否则根据状态码生成
    else
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "else\n\n");
    	
    	status = r->headers_out.status;
    	
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(trace_file_fd, "status=%d\n\n", status);
    	
    	// 2xx 状态码表示请求已成功被服务器接收、理解并处理。这类状态码属于“成功”类别，意味着客户端的请求达到了预期效果。
    	if (status >= NIGHT_HTTP_OK && 
    		status < NIGHT_HTTP_LAST_2XX)
    	{
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "status 2xx\n\n");
        	
			// 204 No Content 请求成功，但响应中不包含任何内容
            if (status == NIGHT_HTTP_NO_CONTENT) 
            {
                r->header_only = 1;
                night_str_null(&r->headers_out.content_type);
                r->headers_out.last_modified_time = -1;
                r->headers_out.last_modified = NULL;
                r->headers_out.content_length = NULL;
                r->headers_out.content_length_n = -1;
            }
            
			// 将状态码转换为相对于 200 的偏移量
			// 作为 ngx_http_status_lines[] 数组的索引。
			// 获取对应的状态行字符串
			// 将其长度累加到总响应头长度 len 中，为后续缓冲区分配做准备
            status -= NIGHT_HTTP_OK;
            status_line = &night_http_status_lines[status];
            len += night_http_status_lines[status].len;
    	}	
    } 
    
    if (status_line)
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "status_line=%s\n\n", status_line->data);
    }
    
    clcf = r->loc_conf[night_http_core_module.ctx_index];
    
    if (r->headers_out.server == NULL)
    {
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "if (r->headers_out.server == NULL)\n\n");
    	
    	len += sizeof(night_http_server_string) - 1;
    	
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "len=%ld\n\n", len);
    } 
    
	// 添加当前时间（使用 ngx_cached_http_time 缓存）
	// 格式：RFC 1123 格式（如 Date: Sun, 02 Nov 2025 12:00:00 GMT）
    if (r->headers_out.date == NULL) 
    {
        len += sizeof("Date: Mon, 28 Sep 1970 06:00:00 GMT" CRLF) - 1;
    }
    
	// Content-Type 头处理
    if (r->headers_out.content_type.len) 
    {
        len += sizeof("Content-Type: ") - 1
               + r->headers_out.content_type.len + 2;
		
		// 只有当原始 content-type 未包含 charset 时才追加
        if (r->headers_out.content_type_len == r->headers_out.content_type.len && 
        	r->headers_out.charset.len)
        {
            len += sizeof("; charset=") - 1 + r->headers_out.charset.len;
        }
    }
    
	// Content-Length 头处理
    if (r->headers_out.content_length == NULL && 
    	r->headers_out.content_length_n >= 0)
    {
        len += sizeof("Content-Length: ") - 1 + NIGHT_OFF_T_LEN + 2;
    }
    
	// Last-Modified 头处理（时间转字符串）
    if (r->headers_out.last_modified == NULL && 
    	r->headers_out.last_modified_time != -1)
    {
        len += sizeof("Last-Modified: Mon, 28 Sep 1970 06:00:00 GMT" CRLF) - 1;
    }
    
	// Connection 头处理
	// 状态码 101 表示服务器同意客户端的协议升级请求
	// 101 响应必须包含 Connection: Upgrade 头，这是 RFC 6455（WebSocket）和 RFC 7230 的强制要求。
	// 这是协议切换的关键信号
    if (r->headers_out.status == NIGHT_HTTP_SWITCHING_PROTOCOLS) 
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "if NIGHT_HTTP_SWITCHING_PROTOCOLS\n\n");
    	
        len += sizeof("Connection: upgrade" CRLF) - 1;
    } 
    
    else if (r->keepalive)
    {
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "else if (r->keepalive)\n\n");
    	
        len += sizeof("Connection: keep-alive" CRLF) - 1;
    }
    else
	{
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "else Connection: close\n\n");
    		
        len += sizeof("Connection: close" CRLF) - 1;
    }
      
	// 遍历用户自定义响应头
    part = &r->headers_out.headers.part;
    header = part->elts;

    for (i = 0; /* void */; i++) 
    {
        if (i >= part->nelts) 
        {
            if (part->next == NULL) 
            {
                break;
            }

            part = part->next;
            header = part->elts;
            i = 0;
        }
		
		// 跳过：hash == 0 表示该头被标记为删除
        if (header[i].hash == 0) 
        {
            continue;
        }

        len += header[i].key.len + sizeof(": ") - 1 + header[i].value.len + sizeof(CRLF) - 1;
    }
    
	// 创建缓冲区
    b = night_create_temp_buf(r->pool, len);
    if (b == NULL) 
    {
        return NIGHT_ERROR;
    }  
    
	//写入状态行
    // "HTTP/1.x "
    b->last = night_cpymem(b->last, "HTTP/1.1 ", sizeof("HTTP/1.x ") - 1);
    
	// status line
    if (status_line) 
    {
        b->last = night_cpymem(b->last, status_line->data, status_line->len);

    } else 
    {
        b->last += sprintf(b->last, "%03d ", status);
    }
    
    *b->last++ = CR; 
    *b->last++ = LF;
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "%s\n\n", b->start);
    
	//  写入 Server 头
    if (r->headers_out.server == NULL) 
    {
		p = night_http_server_string;
		len = sizeof(night_http_server_string) - 1;
        b->last = night_cpymem(b->last, p, len);
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "%s\n\n", b->start);
    
	// 写入 Date 头
    if (r->headers_out.date == NULL) 
    {
        b->last = night_cpymem(b->last, "Date: ", sizeof("Date: ") - 1);
        b->last = night_cpymem(b->last, night_cached_http_time.data, night_cached_http_time.len);

        *b->last++ = CR; 
        *b->last++ = LF;
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "%s\n\n", b->start);
    
    
    // 写入 Content-Type（含 charset）
    if (r->headers_out.content_type.len) 
    {
		b->last = night_cpymem(b->last, "Content-Type: ", sizeof("Content-Type: ") - 1);
		p = b->last;
		b->last = night_cpymem(b->last, r->headers_out.content_type.data, r->headers_out.content_type.len);
		
		if (r->headers_out.content_type_len == r->headers_out.content_type.len && r->headers_out.charset.len)
		{
			b->last = night_cpymem(b->last, "; charset=", sizeof("; charset=") - 1);
            b->last = night_cpymem(b->last, r->headers_out.charset.data, r->headers_out.charset.len);

            // update r->headers_out.content_type for possible logging

            r->headers_out.content_type.len = b->last - p;
            r->headers_out.content_type.data = p;
		}
		
		*b->last++ = CR;
		*b->last++ = LF;
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "%s\n\n", b->start);
    
	// 写入 Content-Length
    if (r->headers_out.content_length == NULL && 
    	r->headers_out.content_length_n >= 0)
    {
        b->last += sprintf(b->last, "Content-Length: %ld" CRLF, r->headers_out.content_length_n);
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "%s\n\n", b->start);
    
	// 写入 Last-Modified
    if (r->headers_out.last_modified == NULL
        && r->headers_out.last_modified_time != -1)
    {
        b->last = night_cpymem(b->last, "Last-Modified: ", sizeof("Last-Modified: ") - 1);
        b->last = night_http_time(b->last, r->headers_out.last_modified_time);

        *b->last++ = CR; 
        *b->last++ = LF;
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "%s\n\n", b->start);
    
	if (r->chunked) 
	{
        b->last = night_cpymem(b->last, "Transfer-Encoding: chunked" CRLF, sizeof("Transfer-Encoding: chunked" CRLF) - 1);
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "%s\n\n", b->start);
    
	if (r->headers_out.status == NIGHT_HTTP_SWITCHING_PROTOCOLS) 
	{
        b->last = night_cpymem(b->last, "Connection: upgrade" CRLF, sizeof("Connection: upgrade" CRLF) - 1);

    }
    else if (r->keepalive) 
    {
        b->last = night_cpymem(b->last, "Connection: keep-alive" CRLF, sizeof("Connection: keep-alive" CRLF) - 1);
    } 
    else 
    {
        b->last = night_cpymem(b->last, "Connection: close" CRLF, sizeof("Connection: close" CRLF) - 1);
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "%s\n\n", b->start);
    
    // 写入所有自定义响应头
    
	part = &r->headers_out.headers.part;
    header = part->elts;

    for (i = 0; /* void */; i++) 
    {

        if (i >= part->nelts) 
        {
            if (part->next == NULL) {
                break;
            }

            part = part->next;
            header = part->elts;
            i = 0;
        }

        if (header[i].hash == 0) 
        {
            continue;
        }

        b->last = night_cpymem(b->last, header[i].key.data, header[i].key.len);
        *b->last++ = ':'; 
        *b->last++ = ' ';

        b->last = night_cpymem(b->last, header[i].value.data, header[i].value.len);
        *b->last++ = CR; 
        *b->last++ = LF;
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "%s\n\n", b->start);
    
	// the end of HTTP header
	// HTTP 规范：头与 body 之间必须有一个空行
    *b->last++ = CR; 
    *b->last++ = LF;
    
    r->header_size = b->last - b->pos;
    
	// 无 body 的情况
    if (r->header_only) 
    {
    	// last_buf：告诉写过滤器这是最后一个数据块
        b->last_buf = 1;
    }
    
	out.buf = b;
    out.next = NULL;

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "return night_http_write_filter(r, &out);\n\n");
	
	// 调用写过滤器发送数据
    return night_http_write_filter(r, &out);
}    
