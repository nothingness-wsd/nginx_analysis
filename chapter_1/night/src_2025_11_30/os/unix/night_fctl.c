#include "night_core.h"
#include "night_fctl.h"

int
night_nonblocking(int fd)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_nonblocking\n\n");
	
	int flags;
	int rc;
	
	flags = fcntl(fd, F_GETFD);
	
	// 添加非阻塞
	rc = fcntl(fd, F_SETFL, flags | O_NONBLOCK); 

    return rc;
}

int
night_async(int fd)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_async\n\n");
    
    int rc;
    int flags;
    
	// 设置拥有者（谁接收 SIGIO）
	rc = fcntl(fd, F_SETOWN, night_pid);
	if (rc == -1)
	{
		return rc;
	}
	
	// 启用 O_ASYNC 标志
	flags = fcntl(fd, F_GETFD);
	rc = fcntl(fd, F_SETFL, flags | O_ASYNC);
	
	return rc;
}

int
night_fdCloexec(int fd)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_fdCloexec\n\n");
    
    int rc;
    
    rc = fcntl(fd, F_SETFD, FD_CLOEXEC);
    
    return rc;
}

