
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <nginx.h>


ngx_int_t   ngx_ncpu;
ngx_int_t   ngx_max_sockets;
ngx_uint_t  ngx_inherited_nonblocking;
ngx_uint_t  ngx_tcp_nodelay_and_tcp_nopush;


struct rlimit  rlmt;


ngx_os_io_t ngx_os_io = {
    ngx_unix_recv,
    ngx_readv_chain,
    ngx_udp_unix_recv,
    ngx_unix_send,
    ngx_udp_unix_send,
    ngx_udp_unix_sendmsg_chain,
    ngx_writev_chain,
    0
};

// 操作系统相关初始化
ngx_int_t
ngx_os_init(ngx_log_t *log)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "ngx_os_init\n\n");
	
    ngx_time_t  *tp;
    ngx_uint_t   n;
#if (NGX_HAVE_LEVEL1_DCACHE_LINESIZE)
    long         size;
#endif

#if (NGX_HAVE_OS_SPECIFIC_INIT)
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "当前平台需要特殊初始化\n" "ngx_os_specific_init\n\n");
	
    if (ngx_os_specific_init(log) != NGX_OK) {
        return NGX_ERROR;
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "当前平台特殊初始化 完成\n" "ngx_os_specific_init completed\n\n");
	
#endif
	
	// 为进程标题（process title）修改功能 初始化环境
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "为进程标题修改功能 初始化环境\n" "ngx_init_setproctitle\n\n");
	
    if (ngx_init_setproctitle(log) != NGX_OK) 
    {
        return NGX_ERROR;
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "为进程标题修改功能 初始化环境 完成\n" "ngx_init_setproctitle completed\n\n");

	// 调用 POSIX 标准函数 getpagesize() 获取系统内存页大小（通常为 4096 字节）
	// 存入全局变量 ngx_pagesize
    ngx_pagesize = getpagesize();
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "ngx_pagesize=%ld\n\n", ngx_pagesize);
    
    // 初始化全局变量 ngx_cacheline_size
    ngx_cacheline_size = NGX_CPU_CACHE_LINE;

	// 计算页大小的以 2 为底的对数, 用于快速计算页对齐
    for (n = ngx_pagesize; n >>= 1; ngx_pagesize_shift++) { /* void */ }
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "ngx_pagesize_shift=%ld\n\n", ngx_pagesize_shift);

#if (NGX_HAVE_SC_NPROCESSORS_ONLN)
	// 获取 CPU 核心数
    if (ngx_ncpu == 0) {
        ngx_ncpu = sysconf(_SC_NPROCESSORS_ONLN);
    }
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, " CPU 核心数\n" "ngx_ncpu=%ld\n\n", ngx_ncpu);
    
#endif

    if (ngx_ncpu < 1) {
        ngx_ncpu = 1;
    }

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, " CPU 核心数\n" "ngx_ncpu=%ld\n\n", ngx_ncpu);
	
#if (NGX_HAVE_LEVEL1_DCACHE_LINESIZE)
	// 获取一级数据缓存行大小
    size = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    if (size > 0) {
        ngx_cacheline_size = size;
    }
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "缓存行大小\n" "ngx_cacheline_size=%ld\n\n", ngx_cacheline_size);
#endif

	// 调用平台相关的 CPU 信息探测函数（如 x86 的 cpuid 指令）
    ngx_cpuinfo();

	//  获取当前进程能打开的最大文件描述符数量
    if (getrlimit(RLIMIT_NOFILE, &rlmt) == -1) {
        ngx_log_error(NGX_LOG_ALERT, log, errno,
                      "getrlimit(RLIMIT_NOFILE) failed");
        return NGX_ERROR;
    }

    ngx_max_sockets = (ngx_int_t) rlmt.rlim_cur;
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "当前进程能打开的最大文件描述符数量\n" "ngx_max_sockets=%ld\n\n", ngx_max_sockets);

#if (NGX_HAVE_INHERITED_NONBLOCK || NGX_HAVE_ACCEPT4)
    ngx_inherited_nonblocking = 1;
#else
    ngx_inherited_nonblocking = 0;
#endif
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "ngx_inherited_nonblocking=%ld\n\n", ngx_inherited_nonblocking);
	
	// 获取当前时间（秒 + 毫秒），返回指向全局时间缓存的指针
    tp = ngx_timeofday();
    
    // 调用 srandom() 初始化标准 C 库的伪随机数生成器（PRNG）
    srandom(((unsigned) ngx_pid << 16) ^ tp->sec ^ tp->msec);

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function ngx_os_init:\t" "return NGX_OK\n\n");

    return NGX_OK;
}


void
ngx_os_status(ngx_log_t *log)
{
    ngx_log_error(NGX_LOG_NOTICE, log, 0, NGINX_VER_BUILD);

#ifdef NGX_COMPILER
    ngx_log_error(NGX_LOG_NOTICE, log, 0, "built by " NGX_COMPILER);
#endif

#if (NGX_HAVE_OS_SPECIFIC_INIT)
    ngx_os_specific_status(log);
#endif

    ngx_log_error(NGX_LOG_NOTICE, log, 0,
                  "getrlimit(RLIMIT_NOFILE): %r:%r",
                  rlmt.rlim_cur, rlmt.rlim_max);
}


#if 0

ngx_int_t
ngx_posix_post_conf_init(ngx_log_t *log)
{
    ngx_fd_t  pp[2];

    if (pipe(pp) == -1) {
        ngx_log_error(NGX_LOG_EMERG, log, ngx_errno, "pipe() failed");
        return NGX_ERROR;
    }

    if (dup2(pp[1], STDERR_FILENO) == -1) {
        ngx_log_error(NGX_LOG_EMERG, log, errno, "dup2(STDERR) failed");
        return NGX_ERROR;
    }

    if (pp[1] > STDERR_FILENO) {
        if (close(pp[1]) == -1) {
            ngx_log_error(NGX_LOG_EMERG, log, errno, "close() failed");
            return NGX_ERROR;
        }
    }

    return NGX_OK;
}

#endif
