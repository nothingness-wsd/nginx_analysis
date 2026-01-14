#include "night_core.h"
#include "night_os.h"
#include "night_linux_init.h"

char		*night_argv_last;
int			night_page_size;

int
night_os_init()
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_os_init\n\n");
    
	if (night_os_specific_init() != NIGHT_OK) 
	{
        return NIGHT_ERROR;
    }
    
    if(night_init_setproctitle() != NIGHT_OK) 
    {
    	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd,"night_init_setproctitle fauled\n\n");
    	
        return NIGHT_ERROR;
    }
    
    night_page_size = getpagesize();
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "night_page_size=%d\n\n", night_page_size);
    
    return NIGHT_OK;
}

int
night_init_setproctitle()
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_init_setproctitle\n\n");
    
    size_t	size;
    int 	i;
    char 	*p;
    
    size = 0;
    
	for (i = 0; environ[i]; i++)
    {
        size += strlen(environ[i]) + 1;
    }
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"size=%ld\n\n", size);
    
    night_argv_last = environ[i-1] + strlen(environ[i-1]);
    
    p = malloc(size);
	if (p == NULL)
    {
    	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(error_log_fd,"malloc(%ld) failed in night_init_setproctitle\n\n", size);
        
        return NIGHT_ERROR;
    }

	for (i = 0; environ[i]; i++)
    {
        size = strlen(environ[i]) + 1;
        memcpy(p,environ[i],size);
        
        environ[i] = p;
        
        p += size;
    }
    
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"environ after move:\n");
    for (i = 0; environ[i]; i++)
    {
    	dprintf(trace_file_fd,"%s\n", environ[i]); 
    }
    dprintf(trace_file_fd,"\n");
    
	return NIGHT_OK;
}
