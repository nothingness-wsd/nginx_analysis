#include "night_core.h"
#include "night_daemon.h"

#define DaemonMaster_trace "trace/DaemonMaster_trace"

int
night_daemon()
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_daemon\n\n");
    
    pid_t 	pid;
    int 	fd;
    int 	rc;
    
    pid = fork();
    
    switch (pid)
    {
    	case -1:
    		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    		dprintf(error_log_fd,"night_daemon fork() failed\n\n");	
    		
    		return NIGHT_ERROR;
    		
    	case 0:	
    		break;
    	
    	default:
    	
    		dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    		dprintf(trace_file_fd,"The parent process exits, leaving only the daemon process\n\n");
    		
    		exit(0);
    }
    
	night_parent_pid = night_pid;
    night_pid = getpid();
    
    rc = setsid();
    if (rc == -1)
    {
    	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd,"setsid() failed\n\n");
    	
    	return NIGHT_ERROR;
    }
    
    umask(0);
    
    trace_file_fd = open(DaemonMaster_trace, O_CREAT|O_TRUNC|O_RDWR, 0666);
    
    fd = open("/dev/null", O_RDWR);
    if (fd == -1)
    {
		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd,"open(\"/dev/null\") failed\n\n");
    	
    	return NIGHT_ERROR;
    }
    
    rc = dup2(fd, STDIN_FILENO);
    if (rc == -1)
    {
    	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd, "dup2(STDIN) failed\n\n");
    	
    	return NIGHT_ERROR;
    }
    
	rc = dup2(fd, STDOUT_FILENO);
    if (rc == -1)
    {
    	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd, "dup2(STDOUT) failed\n\n");
    	
    	return NIGHT_ERROR;
    }
    
    if (fd > STDERR_FILENO)
    {
    	close(fd);
    }
    
    return NIGHT_OK;
}
