
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


ngx_uint_t  ngx_pagesize;
ngx_uint_t  ngx_pagesize_shift;
ngx_uint_t  ngx_cacheline_size;


void *
ngx_alloc(size_t size, ngx_log_t *log)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function:\t" "ngx_alloc\n\n");
	
    void  *p;

    p = malloc(size);
    if (p == NULL) {
        ngx_log_error(NGX_LOG_EMERG, log, ngx_errno,
                      "malloc(%uz) failed", size);
    }

    ngx_log_debug2(NGX_LOG_DEBUG_ALLOC, log, 0, "malloc: %p:%uz", p, size);

	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function ngx_alloc:\t" "return\n\n");
	
    return p;
}


void *
ngx_calloc(size_t size, ngx_log_t *log)
{
    void  *p;

    p = ngx_alloc(size, log);

    if (p) {
        ngx_memzero(p, size);
    }

    return p;
}


#if (NGX_HAVE_POSIX_MEMALIGN)

// 按照指定的对齐字节数分配一块内存
void *
ngx_memalign(size_t alignment, size_t size, ngx_log_t *log)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function:\t" "ngx_memalign\n\n");
	
    void  *p;
    int    err;

	// 调用标准 POSIX 函数 posix_memalign 来分配对齐内存。
	// 第一个参数 &p：传入指针的地址，函数会将分配的内存地址写入 p。
	// 第二个参数 alignment：对齐边界（如 16、32、64 字节等）。
	// 第三个参数 size：请求分配的字节数。
	// 返回值：
	// 成功时返回 0；
	// 失败时返回一个非零的错误码（如 EINVAL、ENOMEM 等）。
    err = posix_memalign(&p, alignment, size);

    if (err) {
        ngx_log_error(NGX_LOG_EMERG, log, err,
                      "posix_memalign(%uz, %uz) failed", alignment, size);
        p = NULL;
    }

    ngx_log_debug3(NGX_LOG_DEBUG_ALLOC, log, 0,
                   "posix_memalign: %p:%uz @%uz", p, size, alignment);

	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function ngx_memalign:\t" "return\n\n");
	
    return p;
}

#elif (NGX_HAVE_MEMALIGN)

void *
ngx_memalign(size_t alignment, size_t size, ngx_log_t *log)
{
    void  *p;

    p = memalign(alignment, size);
    if (p == NULL) {
        ngx_log_error(NGX_LOG_EMERG, log, ngx_errno,
                      "memalign(%uz, %uz) failed", alignment, size);
    }

    ngx_log_debug3(NGX_LOG_DEBUG_ALLOC, log, 0,
                   "memalign: %p:%uz @%uz", p, size, alignment);

    return p;
}

#endif
