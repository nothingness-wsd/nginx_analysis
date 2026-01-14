#include "night_core.h"
#include "night_process_cycle.h"
#include "night_core_module.h"
#include "night_pool.h"
#include "night_cycle.h"
#include "night_proctitle.h"
#include "night_module.h"
#include "night_process.h"
#include "night_time.h"
#include "night_string.h"
#include "night_process_events.h"
#include "night_listening.h"
#include "night_channel.h"
#include "night_event.h"
#include "night_event_timer.h"
#include "night_connection.h"
#include "night_event_posted.h"


char 			master_process[] = "master process";
int 			night_reap = 0;
int				night_worker;
int				night_terminate = 0;
int				night_quit = 0;
int				night_exiting;
int				night_reopen = 0;
night_event_t   night_shutdown_event;

void
night_master_process_cycle()
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_master_process_cycle\n\n");
    
    sigset_t			set;
    size_t 				size;
    int 				i;
    char 				*title;
    char 				*p;
    night_core_conf_t 	*cc;
    int 				live;
    
    sigemptyset(&set);
    
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGQUIT);
    sigaddset(&set, SIGTERM);
    
   	if (sigprocmask(SIG_BLOCK, &set, NULL) == -1) 
	{
		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(error_log_fd,"sigprocmask() failed\n\n");
		
    }
    
    sigemptyset(&set);
    
    size = sizeof(master_process);
    
	for(i = 0; i < night_argc; i++) 
    {
        size += strlen(night_argv[i]) + 1;
    }
    
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"size=%ld\n\n", size);
    
    title = night_pmalloc(night_cycle->pool, size);
    
	if (title == NULL)
    {
    	dprintf(error_log_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd, "night_pmalloc failed for process title while night_master_process_cycle\n\n");
    	
    	// free environ memory
    	if (environ[0])
    	{
    		free(environ[0]);
    	}
    
        exit(2);
    }
    
    p = title;
    
	memcpy(title, master_process, strlen(master_process));
    p += strlen(master_process);
    
    for (i = 0; i < night_argc; i++)
    {
        *p++ = ' ';
        strcat(p, (char*) night_argv[i]);
        p += strlen(night_argv[i]);
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "title=%s\n\n",title);
    
    night_setproctitle(title);
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "night_setproctitle is completed\n\n");
	
	cc = (night_core_conf_t*) night_get_conf(night_cycle->conf_ctx, night_core_module);
    night_start_worker_processes(cc->worker_processes);
    
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"night_start_worker_processes is completed\n\n");
    
	for ( ; ; )
    {
    	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd,"loop of master process\n");
    	dprintf(trace_file_fd,"sigsuspend is waiting signal\n\n");
    	
		sigsuspend(&set);
		
		dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd,"wake up from sigsuspend \n\n");
		
		night_time_update();
		
		live = 1;
		
		if (night_reap) 
		{
            night_reap = 0;
            
            dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            dprintf(trace_file_fd,"reap children\n\n");
            
            live = night_reap_children();
        }
		
		if (!live && (night_terminate || night_quit)) 
		{
            night_master_process_exit();
        }
       
        if (night_terminate) 
        {
			night_signal_worker_processes(SIGTERM);
			continue;
        }

        if (night_quit) 
        {
        	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd,"night_quit = %d\n\n", night_quit);
        	
            night_signal_worker_processes(SIGQUIT);

            continue;
        }
    }
    
}

void
night_start_worker_processes(int n)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_start_worker_processes\n\n");
  
    int i;
    
    for (i = 0; i < n; i++)
    {
        night_spawn_process(night_worker_process_cycle, (void*)(intptr_t)i, "worker process");
        
    }
}

void
night_worker_process_cycle(void *data)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_worker_process_cycle\n\n");
    
    int 	worker;
    char	title[32];
    
    night_memzero(title, 32);
    
    worker = (intptr_t) data;
    
	night_process = NIGHT_PROCESS_WORKER;
	
    night_worker = worker;
    
	sprintf(title, "worker_%d process", worker);
	night_setproctitle(title);
	
	night_worker_process_init(worker);
	
	for( ;; )
	{
		if (night_exiting) 
		{
			dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(trace_file_fd,"process is exiting by night_worker_process_exit\n\n");
			
			night_worker_process_exit();
		}
		
		night_process_events_and_timers();
		
		if (night_terminate) 
		{
			dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(trace_file_fd,"exiting\n\n");
			
            night_worker_process_exit();
        }

        if (night_quit) 
        {
            night_quit = 0;
            
            dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            dprintf(trace_file_fd,"gracefully shutting down\n\n");
            
            night_setproctitle("worker process is shutting down");

			dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(trace_file_fd,"night_exiting=%d\n\n", night_exiting);
			
            if (!night_exiting) 
            {
                night_exiting = 1;
                
                night_set_shutdown_timer();
                night_close_listening_sockets();
                night_close_idle_connections();
                night_event_process_posted(&night_posted_events);
            }
        }
	}  
}    

void
night_master_process_exit()
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_master_process_exit\n\n");
    
    int i = 0;
    
    for (i = 0; night_cycle->modules[i]; i++)
    {
    	if (night_cycle->modules[i]->exit_master)
    	{
			night_cycle->modules[i]->exit_master();
    	}
    }
    
    night_close_listening_sockets();

    night_destroy_pool(night_cycle->pool);

	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"night master process exit(0)\n\n");
    
    // free environ memory
    if (environ[0])
    {
    	free(environ[0]);
    }
    
    exit(0);
}   

void
night_worker_process_init(int worker)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_worker_process_init\n\n");
    
    night_core_conf_t	*cc;
    night_time_t 		*tp;
    int 				n;
    int 				i;
	night_module_t 		*module;
	int 				rc;
    
	cc = (night_core_conf_t*) night_get_conf(night_cycle->conf_ctx, night_core_module);
    
    if ( geteuid() == 0)
    {
        dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(trace_file_fd,"geteuid() == 0\n\n");
        
        if (setgid(cc->gid) == -1) 
        {
        	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            dprintf(error_log_fd,"setgid(%d) failed\n\n",cc->gid);
            
            // fatal
            exit(2);
        }
        
        if (initgroups(cc->username, cc->gid) == -1) 
        {
        	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            dprintf(error_log_fd,"initgroups(%s,%d) failed\n\n",cc->username,cc->gid);
            
            // fatal
            exit(2);
        }
        
        if (setuid(cc->uid) == -1) 
        {
        	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            dprintf(error_log_fd,"setuid(%d) failed\n\n",cc->uid);
            
            // fatal
            exit(2);
        }
        
        dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(trace_file_fd,"Change user completed\n\n");
    }
    
	tp = night_cached_time;
    srandom(((unsigned)night_pid << 16) ^ tp->sec ^ tp->msec);
    
 	for (i = 0; night_cycle->modules[i]; i++) 
    {
    	module = night_cycle->modules[i];
        if (module->init_process) 
        {
            if (module->init_process() != NIGHT_OK) 
            {
				dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            	dprintf(error_log_fd,"%s init_process failed\n\n", module->name);
                
                exit(3);
            }
        }
    }
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "module->init_process() is completed\n\n");
    
    
	for (n = 0; n < night_last_process; n++) 
    {
        if (night_processes[n].pid == -1) 
        {
            continue;
        }

        if (n == night_process_slot) 
        {
            continue;
        }

        if (night_processes[n].channel[1] == -1) 
        {
            continue;
        }

        if (close(night_processes[n].channel[1]) == -1) 
        {
        	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            dprintf(error_log_fd, "close() channel failed\nerrno=%d:\n", errno);
            dprintf(error_log_fd, "%s\n\n", strerror(errno));
            
            exit(4);
        }
    }

    if (close(night_processes[night_process_slot].channel[0]) == -1) 
    {
		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(error_log_fd, "close() channel failed\nerrno=%d:\n", errno);
		dprintf(error_log_fd, "%s\n\n", strerror(errno));
            
		exit(3);
    }
    
    rc = night_add_channel_event(night_channel, NIGHT_READ_EVENT, night_channel_handler);
	if (rc != NIGHT_OK)
    {
    	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd,"night_add_channel_event failed\n\n");
    	
        exit(5);
    }
} 


void 
night_worker_process_exit()
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_worker_process_exit\n\n");
    
    int 				i;
    night_module_t 		**modules;
    night_connection_t 	*c;
    
    modules = night_cycle->modules;
    
    for (i = 0; modules[i]; i++)
    {
    	if (modules[i]->exit_process)
    	{
    		modules[i]->exit_process();
    	}
    }

    night_destroy_pool(night_cycle->pool);
    
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"exit(0);\n\n");
    
    exit(0);
}

void
night_set_shutdown_timer()
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_set_shutdown_timer\n\n");
    
	night_shutdown_event.handler = night_shutdown_timer_handler;
	
    night_shutdown_event.data = night_cycle;
      
	night_shutdown_event.cancelable = 1;

    night_event_add_timer(&night_shutdown_event, 600);
    
}

void
night_shutdown_timer_handler(night_event_t *ev)
{
    int         		i;
    night_connection_t  *c;

    c = night_cycle->connections;

    for (i = 0; i < night_cycle->connection_n; i++) 
    {

        if (c[i].fd == -1
            || c[i].read == NULL
            || c[i].read->accept
            || c[i].read->channel)
        {
            continue;
        }

        c[i].close = 1;
        c[i].error = 1;

        c[i].read->handler(c[i].read);
    }
}

