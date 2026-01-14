#include "night_core.h"
#include "night_signal.h"
#include "night_string.h"
#include "night_process.h"

night_signal_t  signals[] = 
{
    {
		SIGTERM,
		"SIGTERM",
		"stop",
		night_signal_handler 
	},

    {
		SIGQUIT,
		"SIGQUIT",
		"quit",
		night_signal_handler 
	},
	
	{ SIGCHLD, "SIGCHLD", "", night_signal_handler },
      
    { 0, NULL, "", NULL }
};

int
night_init_signals()
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_init_signals\n\n");
  	
  	night_signal_t 		*sig;
  	struct sigaction 	sa;
  	
  	for (sig = signals; sig->signo != 0; sig++)
  	{
  		night_memzero(&sa, sizeof(struct sigaction));
  		
  		if (sig->handler)
  		{
  			sa.sa_flags = SA_SIGINFO;
  			sa.sa_sigaction = sig->handler;
  		}
  		else
  		{
  			sa.sa_handler = SIG_IGN;
  		}
  		
  		sigemptyset(&sa.sa_mask);
  		
  		if (sigaction(sig->signo, &sa, NULL) == -1)
  		{
  			dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
  			dprintf(error_log_fd,"sigaction %s failed \n\n", sig->signame);
  			
  			return NIGHT_ERROR;
  		}
  	}  
    
    return NIGHT_OK;
}

void
night_signal_handler(int signo, siginfo_t *siginfo, void *ucontext)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_signal_handler\n\n");
    
    int 			ignore;
    night_signal_t 	*sig;
    
    ignore = 0;
    
    for (sig = signals; sig->signo != 0; sig++) 
    {
        if (sig->signo == signo) 
        {
            break;
        }
    }
  
  	switch(night_process) 
  	{
  	    case NIGHT_PROCESS_MASTER:
    	case NIGHT_PROCESS_SINGLE:
    		switch (signo) 
    		{
				case SIGQUIT:
            		night_quit = 1;
            		break;

        		case SIGTERM:
        		case SIGINT:
            		night_terminate = 1;
            		break;
            		
				case SIGCHLD:
					night_reap = 1;
					break;
            	
            	default:
            		break;	
    		}
		case NIGHT_PROCESS_WORKER:  
			switch (signo) 
			{
				case SIGQUIT:
            		night_quit = 1;
            		break;

        		case SIGTERM:
        		case SIGINT:
            		night_terminate = 1;
            		break;
            		
            	default:
            		break;	
			} 
		default:
			break;  		
  	}
  	
    if (signo == SIGCHLD) 
    {
        night_process_get_status();
    }

}    

