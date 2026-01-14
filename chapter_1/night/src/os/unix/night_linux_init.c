#include "night_core.h"

char  night_linux_kern_ostype[50];
char  night_linux_kern_osrelease[50];

static night_os_io_t night_linux_io = 
{

};

// 用于在 Linux 平台上完成操作系统相关的初始化工作
int
night_os_specific_init()
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_os_specific_init\n\n");
	
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
    if (uname(&u) == -1) 
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "uname() failed\n");
        dprintf(trace_file_fd, "function night_os_specific_init:\t" "return NIGHT_ERROR\n\n");
        
        return NIGHT_ERROR;
    }

	// 复制操作系统类型（sysname → night_linux_kern_ostype）
    (void) night_cpystrn(night_linux_kern_ostype, (char *) u.sysname, sizeof(night_linux_kern_ostype));

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "night_linux_kern_ostype=%s\n\n", night_linux_kern_ostype);
	
	// 复制内核版本（release → night_linux_kern_osrelease）
    (void) night_cpystrn(night_linux_kern_osrelease, (char *) u.release, sizeof(night_linux_kern_osrelease));

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "night_linux_kern_osrelease=%s\n\n", night_linux_kern_osrelease);
	
	// night_os_io：
	// 全局函数指针结构体（如 ngx_os_io_t 类型），定义了 OS 相关的 I/O 操作（如 sendfile、read、write、recv 等）。
	// night_linux_io：
	// 在 Linux 平台下定义的 I/O 函数表（包含对 epoll、sendfile、TCP_FASTOPEN 等特性的支持）。
	// 赋值含义：
	// 将 Linux 特定的 I/O 方法注册到全局接口 night_os_io。
	// 后续网络模块（如 event 模块）会通过 night_os_io 调用底层 I/O 函数，实现跨平台兼容。
	// 抽象操作系统差异的关键设计。通过 night_os_io，上层代码无需关心底层是 Linux、FreeBSD 还是 macOS。

    night_os_io = night_linux_io;

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function night_os_specific_init:\t" "return NIGHT_OK\n\n");
	
    return NIGHT_OK;
}


