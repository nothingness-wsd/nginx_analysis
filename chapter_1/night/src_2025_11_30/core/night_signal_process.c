#include "night_core.h"
#include "night_signal_process.h"
#include "night_file.h"
#include "night_string.h"
#include "night_signal.h"


int
night_signal_process(char *s)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_signal_process\n");
    dprintf(trace_file_fd,"signal process started\n\n");
 	
	night_file_t 			file;
	ssize_t 				n;
	char 					buf[NIGHT_INT64_STRLEN + 2];
	pid_t 					pid;
 	
 	night_memzero(&file, sizeof(night_file_t));
 	
 	file.filename.len = strlen(NIGHT_PIDFILE);
 	file.filename.data = NIGHT_PIDFILE;
 	
 	file.fd = open(file.filename.data, O_RDONLY, 0666);
 	
    if (file.fd == NIGHT_INVALID_FD)
    {
    	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd,"open(\"%s\") failed \n\n", file.filename.data);
    	
        return 1;
    }
    
    n = night_read_file(&file, buf, NIGHT_INT64_STRLEN + 2, 0);
    
	if (close(file.fd) == -1) 
	{
		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(error_log_fd,"close %s failed\n\n", file.filename.data);
		
    }  
    
	if (n == NIGHT_ERROR) 
	{
		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(error_log_fd,"read pidfile %s failed\n\n", file.filename.data);
		
        return 1;
    } 
    
    while(n-- && (buf[n] == CR || buf[n] == LF)) 
    { /* void */ }
        
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"n=%ld\n", n);
    dprintf(trace_file_fd,"%s\n\n", buf);
    
    pid = night_atoi(buf, ++n);
    if (pid == NIGHT_ERROR)
    {
    	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd,"invalid PID number \"%s\" in \"%s\" \n\n", buf, file.filename.data);
    	
    	return 1;
    }
    
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"pid=%d\n\n", pid);
    
    return night_os_signal_process(s, pid);
}

int
night_os_signal_process(char *name, pid_t pid)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_os_signal_process\n\n");
    
    night_signal_t *sig;

    for (sig = signals; sig->signo != 0; sig++) 
    {
        if (strcmp(name, sig->name) == 0) 
        {
            if (kill(pid, sig->signo) != -1) 
            {
            	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            	dprintf(trace_file_fd,"kill(%d, %d)\n\n", pid, sig->signo);
            	
                return 0;
            }

			dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(error_log_fd,"kill(%d, %d) failed\n\n", pid, sig->signo);
			
        }
    }

    return 9;
}    
