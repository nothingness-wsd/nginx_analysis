#include "night_core.h"

#define NIGHT_CPU_CACHE_LINE 64

night_os_io_t  		night_os_io = 
{

};

size_t  		night_pagesize = 4096;
size_t			night_cacheline_size;
size_t			night_pagesize_shift;
size_t			night_ncpu;
size_t 			night_max_sockets;

struct rlimit  	rlmt;


// 操作系统相关初始化
int
night_os_init()
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_os_init\n\n");

	size_t 			n;
	size_t 			size;
	night_time_t  	*tp;
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "当前平台需要特殊初始化\n" "night_os_specific_init\n\n");
	
    if (night_os_specific_init() != NIGHT_OK) 
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "night_os_specific_init failed\n");
    	dprintf(trace_file_fd, "function night_os_init:\t" "return NIGHT_ERROR\n\n");
    	
        return NIGHT_ERROR;
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "当前平台特殊初始化 完成\n" "night_os_specific_init completed\n\n");
	
	// 为进程标题（process title）修改功能 初始化环境
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "为进程标题修改功能 初始化环境\n" "night_init_setproctitle\n\n");
	
    if (night_init_setproctitle() != NIGHT_OK) 
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "function night_os_init:\t" "return NIGHT_ERROR\n\n");
    	
        return NIGHT_ERROR;
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "为进程标题修改功能 初始化环境 完成\n" "night_init_setproctitle completed\n\n");
	
	// 调用 POSIX 标准函数 getpagesize() 获取系统内存页大小（通常为 4096 字节）
	// 存入全局变量 night_pagesize
    night_pagesize = getpagesize();
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "night_pagesize=%ld\n\n", night_pagesize);
    
	// 初始化全局变量 night_cacheline_size
    night_cacheline_size = NIGHT_CPU_CACHE_LINE;
    
	// 计算页大小的以 2 为底的对数
    for (n = night_pagesize; n >>= 1; night_pagesize_shift++) { /* void */ }
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "night_pagesize_shift=%ld\n\n", night_pagesize_shift);
    
	// 获取 CPU 核心数
    if (night_ncpu == 0) 
    {
        night_ncpu = sysconf(_SC_NPROCESSORS_ONLN);
    }
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "CPU 核心数\n" "night_ncpu=%ld\n\n", night_ncpu);

    if (night_ncpu < 1) 
    {
        night_ncpu = 1;
    }

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "CPU 核心数\n" "night_ncpu=%ld\n\n", night_ncpu);
    
	// 获取一级数据缓存行大小
    size = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    if (size > 0) 
    {
        night_cacheline_size = size;
    }
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "缓存行大小\n" "night_cacheline_size=%ld\n\n", night_cacheline_size);
    
	//  获取当前进程能打开的最大文件描述符数量
    if (getrlimit(RLIMIT_NOFILE, &rlmt) == -1) 
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "getrlimit(RLIMIT_NOFILE) failed\n");
    	dprintf(trace_file_fd, "function night_os_init:\t" "return NIGHT_ERROR\n\n");
    	
        return NIGHT_ERROR;
    }

    night_max_sockets =  rlmt.rlim_cur;
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "当前进程能打开的最大文件描述符数量\n" "night_max_sockets=%ld\n\n", night_max_sockets);
    
	// 获取当前时间（秒 + 毫秒），返回指向全局时间缓存的指针
    tp = (night_time_t *) night_cached_time;
    
    // 调用 srandom() 初始化标准 C 库的伪随机数生成器（PRNG）
    srandom(((unsigned) night_pid << 16) ^ tp->sec ^ tp->msec);

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function night_os_init:\t" "return NIGHT_OK\n\n");

    return NIGHT_OK;
}
