#include "night_core.h"
#include "night_process.h"
#include "night_fctl.h"
#include "night_string.h"
#include "night_channel.h"

int 			night_last_process = 0;
night_process_t night_processes[NIGHT_MAX_PROCESSES];
int 			night_channel;
int				night_process_slot;

pid_t
night_spawn_process(night_spawn_proc_pt proc, void *data, char *name)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_spawn_process\n\n");
    
    pid_t 	pid;
    int 	s;
    int 	rc;
    char	tracefile[32];
    char 	worker_name[32];
    
    night_memzero(tracefile, 32);
    night_memzero(worker_name, 32);
    
    for (s = 0; s < night_last_process; s++)
    {
		if (night_processes[s].pid == -1) 
        {
			break;
        }
    }
    
	if (s == NIGHT_MAX_PROCESSES)
    {
    	dprintf(error_log_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(error_log_fd, "no more than %d processes can be spawned\n\n", NIGHT_MAX_PROCESSES);
        
        return NIGHT_INVALID_PID;
    }
    
    rc = socketpair(AF_UNIX, SOCK_STREAM, 0, night_processes[s].channel);
    if (rc == -1)
    {
    	dprintf(error_log_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd, "socketpair() failed while spawning \"%s %ld\"\n\n", name, (intptr_t)data);
    	
    	return NIGHT_INVALID_PID;
    }
    
    rc = night_nonblocking(night_processes[s].channel[0]);
	if (rc == -1)
    {
    	dprintf(error_log_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(error_log_fd,"night_nonblocking failed while spawning \"%s %ld\"\n\n", name, (intptr_t)data);
        
        close(night_processes[s].channel[0]);
        close(night_processes[s].channel[1]);
        
        return NIGHT_INVALID_PID;
    }
    
	rc = night_nonblocking(night_processes[s].channel[1]);
	if (rc == -1)
    {
    	dprintf(error_log_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(error_log_fd,"night_nonblocking failed while spawning \"%s %ld\"\n\n", name, (intptr_t)data);
        
        close(night_processes[s].channel[0]);
        close(night_processes[s].channel[1]);
        
        return NIGHT_INVALID_PID;
    }
    
    rc = night_async(night_processes[s].channel[0]);
	if (rc == -1)
    {
    	dprintf(error_log_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(error_log_fd,"night_async failed while spawning \"%s %ld\"\n\n", name, (intptr_t)data);
        
        close(night_processes[s].channel[0]);
        close(night_processes[s].channel[1]);
        
        return NIGHT_INVALID_PID;
    }
    

	rc = night_fdCloexec(night_processes[s].channel[0]);
	if (rc == -1) 
    {
    	dprintf(error_log_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(error_log_fd,"night_fdCloexec failed while spawning "
        						"\"%s %ld\"\n\n", name, (intptr_t)data);
        
        close(night_processes[s].channel[0]);
        close(night_processes[s].channel[1]);
        
        return NIGHT_INVALID_PID;
    }
    
	rc = night_fdCloexec(night_processes[s].channel[1]);
	if(rc == -1) 
    {
    	dprintf(error_log_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(error_log_fd,"night_fdCloexec failed while spawning " 
        						"\"%s %ld\"\n\n", name, (intptr_t)data);
        
        close(night_processes[s].channel[0]);
        close(night_processes[s].channel[1]);
        
        return NIGHT_INVALID_PID;
    }
    
    night_channel = night_processes[s].channel[1];
    
    night_process_slot = s;
    
	pid = fork();
    
    switch(pid)
    {
    	case -1:
    		dprintf(error_log_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    		dprintf(error_log_fd,"fork() failed while spawning \"%s %ld\"\n\n", name, (intptr_t)data);
            
			close(night_processes[s].channel[0]);
			close(night_processes[s].channel[1]);
			
            return NIGHT_INVALID_PID;
    		
    	case 0:
			night_parent_pid = night_pid;
            night_pid = getpid();
            
            sprintf(tracefile, "trace/worker_%ld", (intptr_t) data);
            trace_file_fd = open(tracefile, O_CREAT|O_TRUNC|O_RDWR, 0666);
            
            dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            dprintf(trace_file_fd,"worker_%ld start:\n\n", (intptr_t)data);
            
            proc(data);
        
    		break;
    		
    	default:
    	
    		dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    		dprintf(trace_file_fd,"spawn_process is completed\n\n");
    		
    		break;
    } 
    
    night_processes[s].pid = pid;
    night_processes[s].exited = 0;
	night_processes[s].exiting = 0;
	night_processes[s].status = 0;
	night_processes[s].detached = 0;
	
	sprintf(worker_name, "%s_%ld", name, (intptr_t) data);
	night_processes[s].name = worker_name;
    
	if (s == night_last_process) 
	{
        night_last_process++;
        
        dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        dprintf(trace_file_fd,"night_last_process=%d\n\n" , night_last_process );
    }

    return pid;
}

void
night_process_get_status(void)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_process_get_status\n\n");
    
	pid_t 	pid;
	int 	status;
	int 	one = 0;
	int 	i = 0;
	char 	*process;
        
    for( ;; )
    {
		pid = waitpid(-1, &status, WNOHANG);
		
		if (pid == 0) 
		{
            return;
        }
        
        if (pid == -1) 
        {
			if (errno == EINTR) 
			{
                continue;
            }
            
			if (errno == ECHILD && one) 
			{
                return;
            }
            
			if (errno == ECHILD) 
			{
				dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
				dprintf(error_log_fd,"waitpid() failed for ECHILD\n\n");
				
                return;
            }
            
            dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            dprintf(error_log_fd,"waitpid() failed\n\n");
            
            return;
        }
        
        one = 1;
        process = "unknown process";
        
		for (i = 0; i < night_last_process; i++) 
		{
            if (night_processes[i].pid == pid) 
            {
                night_processes[i].status = status;
                night_processes[i].exited = 1;
                process = night_processes[i].name;
                
                break;
            }
        }
        
        dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(trace_file_fd, "%s is exited\n\n" , process);
        
    }
}

void
night_signal_worker_processes(int signo)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_signal_worker_processes\n\n");
    
    night_channel_t ch;
    int 			i;
    int 			rc;
    
    night_memzero(&ch, sizeof(night_channel_t));
    
    ch.command = 0;
    
    switch (signo)
    {
		case SIGQUIT:
			ch.command = NIGHT_CMD_QUIT;
        	break;

		case SIGTERM:
			ch.command = NIGHT_CMD_TERMINATE;
			break;
			
		default:
			ch.command = 0;		
    }
    
    ch.fd = -1;
    
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"night_last_process=%d\n\n", night_last_process);
    
    for (i = 0; i < night_last_process; i++) 
    {
    	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd,"night_processes[%d].pid=%d\n\n", i, night_processes[i].pid);
    	
		if (night_processes[i].pid == -1) 
		{
            continue;
        }
        
        dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(trace_file_fd,"night_processes[%d].exiting = %d\n", i, night_processes[i].exiting);
        dprintf(trace_file_fd,"signo=%d\n\n", signo);
        
		if (night_processes[i].exiting && signo == SIGQUIT)
        {
            continue;
        }
        
        dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(trace_file_fd,"ch.command=%d\n\n", ch.command);
        
		if (ch.command) 
		{
			
			rc = night_write_channel(night_processes[i].channel[0], &ch, sizeof(night_channel_t));
			
			dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(trace_file_fd,"%d=night_write_channel\n\n", rc);
			
            if (rc == NIGHT_OK)
            {
				night_processes[i].exiting = 1;

                continue;
            }
            
            rc = kill(night_processes[i].pid, signo);
            
			dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(trace_file_fd,"%d=kill(%d, %d)\n\n", rc, night_processes[i].pid, signo);
            
			if(rc == -1) 
        	{
				dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
				dprintf(error_log_fd,"kill(%d, %d) failed:\n", night_processes[i].pid, signo);
				dprintf(error_log_fd,"errno=%d:\n%s\n\n",errno, strerror(errno));

            	continue;
        	}
        	
        	night_processes[i].exiting = 1;
        }
    }
}  

int
night_reap_children()
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_reap_children\n\n");
    
    int 			live = 0;
    int 			i = 0;
    night_channel_t ch;
    int 			n = 0;
    
    for (i = 0; i < night_last_process; i++)
    {
		if (night_processes[i].pid == -1) 
		{
            continue;
        }
        
        if (night_processes[i].exited) 
        {
        	if (!night_processes[i].detached) 
        	{
        		night_close_channel(night_processes[i].channel);
        		
				night_processes[i].channel[0] = -1;
                night_processes[i].channel[1] = -1;
                
				ch.pid = night_processes[i].pid;
                ch.slot = i;
                
                for (n = 0; n < night_last_process; n++) 
                {
					if (night_processes[n].exited
                        || night_processes[n].pid == -1
                        || night_processes[n].channel[0] == -1)
                    {
                        continue;
                    }
                    
					night_write_channel(night_processes[n].channel[0], &ch, sizeof(night_channel_t));
                }
                
                night_processes[i].detached = 1;
        	}
        	
			if (i == night_last_process - 1) 
			{
                night_last_process--;
                night_processes[i].pid = -1;
            } 
            else 
            {
				night_processes[i].pid = -1;
            }
        }
        else if (night_processes[i].exiting || !night_processes[i].detached) 
        {
            live = 1;
        }
    }
    
    return live;
}
