#include "night_core.h"
#include "night_pidfile.h"

int
night_create_pidfile(night_str_t *pidfile)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_create_pidfile\n\n");
    
    int 			len;
    char 			pid[NIGHT_INT64_STRLEN + 1];
    int 			rc;
    int				fd;
    
    fd = open(pidfile->data, O_CREAT|O_TRUNC|O_RDWR, 0666);
    if (fd == -1)
    {
    	dprintf(error_log_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd, "open(%s) failed while night_create_pidfile\n\n", pidfile->data);
    	
    	return NIGHT_ERROR;
    }
    
    len = snprintf(pid, NIGHT_INT64_STRLEN + 1,"%ld\n\n", (long)night_pid);
    
    if (write(fd, pid, len) == -1)
    {
		dprintf(error_log_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd, "write pid to pidfile failed\n\n");
    	
    	close(fd);
    	return NIGHT_ERROR;
    }
    
    close(fd);
    return NIGHT_OK;
}
