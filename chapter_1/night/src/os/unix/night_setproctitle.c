#include "night_core.h"

static char *night_os_argv_last;

// 主要作用是为后续修改进程标题（process title）提供支持
int
night_init_setproctitle()
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_init_setproctitle\n\n");
	
	size_t		size;
	char		*p;
	int			i;
	
	// 初始化 size 为 0，用于累加所有环境变量字符串的总长度（包括结尾的 '\0'）
    size = 0;
    
	// 遍历全局变量 environ
	// 对每个环境变量字符串，计算其长度（strlen）并加 1（用于 '\0' 结束符），累加到 size
	// 目的：计算复制所有环境变量所需的最大内存总量。
    for (i = 0; environ[i]; i++) 
    {
        size += strlen(environ[i]) + 1;
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "size=%ld\n\n", size);
	
	// 分配 size 字节的内存
    p = malloc(size);
    if (p == NULL) 
    {
        return NIGHT_ERROR;
    }
    
	// night_os_argv_last 是一个全局指针（u_char *），用于标记 argv 区域的结束位置。
	// 初始将其设为 argv[0] 的起始地址
    night_os_argv_last = night_os_argv[0];
    
	// 遍历 argv 数组
    for (i = 0; night_os_argv[i]; i++) 
    {
        if (night_os_argv_last == night_os_argv[i]) 
        {
            night_os_argv_last = night_os_argv[i] + strlen(night_os_argv[i]) + 1;
        }
    }
    
	// 再次遍历 environ 数组
	// 检查当前环境变量 environ[i] 的地址是否正好等于 night_os_argv_last。
	// 如果是，说明该环境变量紧跟在 argv 区域之后，属于“连续内存区域”的一部分。
	// 只有这些“紧邻 argv 的环境变量”才需要被复制，因为后续 setproctitle 会覆盖从 argv[0] 开始到 night_os_argv_last 的内存。
    for (i = 0; environ[i]; i++) 
    {
        if (night_os_argv_last == environ[i]) 
        {
            size = strlen(environ[i]) + 1;
            night_os_argv_last = environ[i] + size;

            night_cpystrn(p, (char *) environ[i], size);
            environ[i] = (char *) p;
            p += size;
        }
    }
    
	// 指向整个 argv+environ 连续区域的最后一个字节（通常是 '\0'）
    night_os_argv_last--;
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function night_init_setproctitle:\t" "return NIGHT_OK\n\n");
	
    return NIGHT_OK;
}

