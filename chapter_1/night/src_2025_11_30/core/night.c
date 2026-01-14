#include "night_core.h"
#include "night.h"
#include "night_time.h"
#include "night_proctitle.h"
#include "night_modules.h"
#include "night_cycle.h"
#include "night_string.h"
#include "night_daemon.h"
#include "night_core_module.h"
#include "night_module.h"
#include "night_pidfile.h"
#include "night_process_cycle.h"
#include "night_signal.h"
#include "night_signal_process.h"
#include "night_os.h"

#define Master_trace "trace/Master_trace"
#define error_log "log/error_log"

int 			trace_file_fd;
int 			error_log_fd;

char 			*night_signal;
int 			night_process;

int 			night_pid;
int 			night_parent_pid;

int 			night_argc;
char			**night_argv;

night_cycle_t 	*night_cycle;

int main(int argc, char **argv)
{
    trace_file_fd = open(Master_trace,O_CREAT|O_TRUNC|O_RDWR,0666);
    error_log_fd = open(error_log,O_CREAT|O_TRUNC|O_RDWR,0666);
    
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "main\n\n");
    
    int 				rc;
    night_cycle_t 		cycle;
    night_core_conf_t	*cc;
    
	// get cmd options
    rc = night_get_options(argc, argv);
	if (rc != NIGHT_OK) 
	{
        dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(error_log_fd,"night_get_options failed\n\n");
        
        return 1;
    }
    
    // night -s
	if (night_signal) 
	{    
        rc = night_signal_process(night_signal);
        
        dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(trace_file_fd,"night_signal_process is completed and return rc=%d\n\n", rc);
        
        return rc;
    }
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"night_signal is completed\n\n");
    
	// pid
    night_pid = getpid();
    night_parent_pid = getppid();
    
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"night_pid=%d\n", night_pid);
    dprintf(trace_file_fd,"night_parent_pid=%d\n\n", night_parent_pid);
    
    // save as global variable
	night_argc = argc;
    night_argv = argv;
    
	// time init
    rc = night_time_init();
    if (rc == NIGHT_ERROR)
    {
		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(error_log_fd,"night_time_init failed\n\n");
        
        return 1;
    }
    
	// os init
    rc = night_os_init();
    if (rc != NIGHT_OK) 
    {
    	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd,"night_os_init failed\n\n" );
    	
        return 1;
    }
    
    night_preinit_modules();
    
    night_memzero(&cycle, sizeof(night_cycle_t));
    
    night_cycle = &cycle;
    
    // init cycle
	rc = night_init_cycle();
	if (rc != NIGHT_OK) 
	{
		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd,"night_init_cycle failed\n\n" );
    	
        return 1;
	}
	
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"night_init_cycle completed\n\n");
	
	
/*
	// night -s
	if(night_signal) 
	{    
        rc = night_signal_process(night_signal);
        dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(trace_file_fd,"night_signal_process is completed and return rc=%d\n\n", rc);
        return rc;
    }
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"night_signal is completed\n\n");
*/
    
    night_process = NIGHT_PROCESS_MASTER;
    
	rc = night_daemon();
	if (rc != NIGHT_OK) 
	{
		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd,"night_daemon failed\n\n" );
		
		return 1;
    }
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"The daemon process is completed\n\n");
    
    // create pidfile to record pid
	cc = (night_core_conf_t*) night_get_conf(night_cycle->conf_ctx, night_core_module);
    
    rc = night_create_pidfile(&cc->pidfile);
    if (rc != NIGHT_OK) 
    {
        return 1;
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "create pidfile completed\n\n");
	
	// set signal handler 
	rc = night_init_signals();
	if(rc != NIGHT_OK) 
	{
        return 1;
    }
    
	// start master process and worker process
	night_master_process_cycle(&cycle);
	
    // free environ memory
    if (environ[0])
    {
    	free(environ[0]);
    }
    
    return 0;
}

int
night_get_options(int argc, char **argv)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_get_options\n\n");
    
    int i;
    char *p;
    
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"argc=%d\n\n", argc);
    
    for(i = 1; i < argc; i++)
    {
    	p = (char*) argv[i];
    	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd,"argv[%d]=%s\n\n", i, p);
    	
    	if (*p++ != '-')
    	{
			dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    		dprintf(error_log_fd,"invalid option: \"%s\", need \'-\'\n\n", argv[i]);
    		
    		return NIGHT_ERROR;
    	}
    	
    	while (*p)
    	{
    		switch (*p++)
    		{
    			case 's':
    				if (*p)
    				{
    					night_signal = (char*) p;
    				} 
					else if (argv[++i]) 
                	{
                    	night_signal = argv[i];
                	}
                	else
                	{
						dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
                		dprintf(error_log_fd,"option \"-s\" requires parameter\n\n");
                    	
                    	return NIGHT_ERROR;
                	}
                	
                	if (strcmp(night_signal, "stop") == 0 	||
                		strcmp(night_signal, "quit") == 0 	||
                		strcmp(night_signal, "reopen") == 0 ||
                		strcmp(night_signal, "reload") == 0		)
                	{
                		dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
                		dprintf(trace_file_fd,"argv[%d]=%s", i, night_signal);
                		
						night_process = NIGHT_PROCESS_SIGNALLER;
                    	goto next;
                	}
                	
					dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
					dprintf(error_log_fd,"invalid option: \"%s\"\n\n", night_signal);
               
                	return NIGHT_ERROR;
                	
                default:
					dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            		dprintf(error_log_fd,"invalid option: \"%c\"", *(p - 1));
            		
            		return NIGHT_ERROR;
                	
    		}
    	}
    	next:
    		continue;
    	
    }
    
    return NIGHT_OK;
}

