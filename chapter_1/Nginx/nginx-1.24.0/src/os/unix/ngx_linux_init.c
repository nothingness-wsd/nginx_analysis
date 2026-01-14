
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


u_char  ngx_linux_kern_ostype[50];
u_char  ngx_linux_kern_osrelease[50];


static ngx_os_io_t ngx_linux_io = {
    ngx_unix_recv,
    ngx_readv_chain,
    ngx_udp_unix_recv,
    ngx_unix_send,
    ngx_udp_unix_send,
    ngx_udp_unix_sendmsg_chain,
#if (NGX_HAVE_SENDFILE)
    ngx_linux_sendfile_chain,
    NGX_IO_SENDFILE
#else
    ngx_writev_chain,
    0
#endif
};

// 用于在 Linux 平台上完成操作系统相关的初始化工作
ngx_int_t
ngx_os_specific_init(ngx_log_t *log)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "ngx_os_specific_init\n\n");
	
	// POSIX 标准结构体，用于存储系统信息（定义在 <sys/utsname.h> 中）
	// 成员包括：
	// sysname：操作系统名称（如 "Linux"）
	// nodename：网络节点名（主机名）
	// release：内核发布版本（如 "5.15.0-91-generic"）
	// version：内核编译版本信息
	// machine：硬件架构（如 "x86_64"）
	// 变量 u：用于接收 uname() 系统调用返回的信息。
    struct utsname  u;

	// 调用 uname() 获取系统信息
    if (uname(&u) == -1) {
        ngx_log_error(NGX_LOG_ALERT, log, ngx_errno, "uname() failed");
        return NGX_ERROR;
    }

	// 复制操作系统类型（sysname → ngx_linux_kern_ostype）
    (void) ngx_cpystrn(ngx_linux_kern_ostype, (u_char *) u.sysname,
                       sizeof(ngx_linux_kern_ostype));

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "ngx_linux_kern_ostype=%s\n\n", ngx_linux_kern_ostype);
	
	// 复制内核版本（release → ngx_linux_kern_osrelease）
    (void) ngx_cpystrn(ngx_linux_kern_osrelease, (u_char *) u.release,
                       sizeof(ngx_linux_kern_osrelease));

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "ngx_linux_kern_osrelease=%s\n\n", ngx_linux_kern_osrelease);
	
	// ngx_os_io：
	// 全局函数指针结构体（如 ngx_os_io_t 类型），定义了 OS 相关的 I/O 操作（如 sendfile、read、write、recv 等）。
	// ngx_linux_io：
	// 在 Linux 平台下定义的 I/O 函数表（包含对 epoll、sendfile、TCP_FASTOPEN 等特性的支持）。
	// 赋值含义：
	// 将 Linux 特定的 I/O 方法注册到全局接口 ngx_os_io。
	// 后续网络模块（如 event 模块）会通过 ngx_os_io 调用底层 I/O 函数，实现跨平台兼容。
	// 意义：这是 Nginx 抽象操作系统差异的关键设计。通过 ngx_os_io，上层代码无需关心底层是 Linux、FreeBSD 还是 macOS。

    ngx_os_io = ngx_linux_io;

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function ngx_os_specific_init:\t" "return NGX_OK\n\n");
	
    return NGX_OK;
}


void
ngx_os_specific_status(ngx_log_t *log)
{
    ngx_log_error(NGX_LOG_NOTICE, log, 0, "OS: %s %s",
                  ngx_linux_kern_ostype, ngx_linux_kern_osrelease);
}
