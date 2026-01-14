#include "night_core.h"
#include "night_event_core_module.h"
#include "night_module.h"
#include "night_event_module_ctx.h"
#include "night_pool.h"
#include "night_cycle.h"
#include "night_conf.h"
#include "night_string.h"
#include "night_event.h"
#include "night_connection.h"
#include "night_event_posted.h"
#include "night_event_timer.h"
#include "night_listening.h"
#include "night_event_accept.h"
#include "night_event_udp.h"

night_command_t night_event_core_commands[] = 
{
	{
		night_string("worker_connections"),
		NIGHT_EVENT_CONF,
		0,
		night_event_connections
	},
	night_null_command
};

night_event_module_ctx_t night_event_core_module_ctx = 
{
	night_event_core_create_conf,
	night_event_core_init_conf,
	{NULL}
};
	
night_module_t night_event_core_module = 
{
	"night_event_core_module",
	NIGHT_MODULE_UNSET_INDEX,
	NIGHT_MODULE_UNSET_INDEX,
	NIGHT_EVENT_MODULE,
	night_event_core_commands,
	&night_event_core_module_ctx,
	night_event_core_process_init,
	NULL,
	NULL
};

night_event_actions_t 	night_event_actions;

void*
night_event_core_create_conf()
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function:\t" "night_event_core_create_conf\n\n");
	
	night_event_core_conf_t* ecf;
    
    ecf = night_pmalloc( night_cycle->pool, sizeof(night_event_core_conf_t));
    if( ecf == NULL )
    {
    	dprintf(error_log_fd,"night_pmalloc failed to allocate memory for night_event_conf_t while night_event_core_create_conf\n\n");
        return NULL;
    }
    
    ecf->connections = NIGHT_CONF_INT_UNSET;
    ecf->use = NIGHT_CONF_INT_UNSET;
    
    return ecf;
}

int		
night_event_connections(night_conf_t *cf, night_command_t *cmd, void *conf)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_event_connections\n\n");
	
	night_event_core_conf_t 	*ecf;
	night_str_t					*value;
	
	ecf = (night_event_core_conf_t*) conf;
	
	if(ecf->connections != NIGHT_CONF_INT_UNSET)
	{
		dprintf(error_log_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(error_log_fd, "worker_connections configuration is duplicate\n\n");
		
		return NIGHT_ERROR;
	}
	
	value = cf->args.elts;
	
	ecf->connections = night_atoi(value[1].data, value[1].len);
	if(ecf->connections == NIGHT_ERROR)
	{
		dprintf(error_log_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(error_log_fd, "worker_connections configuration's value is invalid\n\n");
		
		return NIGHT_ERROR;
	}
	
	night_cycle->connection_n = ecf->connections;
	
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"ecf->connections=%d\n\n",ecf->connections);

    return NIGHT_OK;
}


int
night_event_core_init_conf(void *conf)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function:\t" "night_event_core_init_conf\n\n");
	
    night_event_core_conf_t *ecf = conf;
    
	night_cycle->connection_n = ecf->connections;
    ecf->use = night_epoll_module.ctx_index;
    
	return NIGHT_OK;
}

int
night_event_core_process_init()
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"function:\t" "night_event_core_process_init\n\n");
    
    night_event_core_conf_t 	*ecf;
    int 						m;
	night_event_module_ctx_t  	*module_ctx;
    night_module_t 				*module;
    int							rc;
    struct rlimit 				rlim;
	night_event_t 				*rev;
    night_event_t 				*wev;
    night_connection_t 			*c;
    int 						i;
    night_connection_t 			*next;
    night_listening_t 			*ls;
    
    night_queue_init(&night_posted_next_events);
    night_queue_init(&night_posted_accept_events);
    night_queue_init(&night_posted_events);
    
    dprintf(trace_file_fd, "FILE = %s:LINE = %d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd, "night_posted_next_events, night_posted_accept_events, night_posted_events " 
    						"init completed\n\n");
    
	if (night_event_timer_init() != NIGHT_OK) 
	{
        return NIGHT_ERROR;
    }
    
    ecf = night_get_event_conf(night_cycle->conf_ctx, night_event_core_module);
	for (m = 0; night_cycle->modules[m]; m++) 
	{
        if (night_cycle->modules[m]->type != NIGHT_EVENT_MODULE) 
        {
            continue;
        }

        if (night_cycle->modules[m]->ctx_index != ecf->use) 
        {
            continue;
        }

		module = night_cycle->modules[m];
        module_ctx = night_cycle->modules[m]->ctx;
        
        dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        dprintf(trace_file_fd,"night_cycle->modules[%d]->name=%s\n\n" , m , module->name );
        
        if (module_ctx->actions.init() != NIGHT_OK) 
        {
        	dprintf(error_log_fd, "FILE = %s:LINE = %d\n" , __FILE__, __LINE__ );
        	dprintf(error_log_fd, "actions.init() failed\n\n" );
        	
            // fatal
            exit(2);
        }

        break;
    }
    
    rc = getrlimit(RLIMIT_NOFILE, &rlim);
    
	if (rc == -1) 
	{
		dprintf(error_log_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(error_log_fd, "getrlimit(RLIMIT_NOFILE) failed");
		
		return NIGHT_ERROR;
	}
        
    night_cycle->files_n = (uint64_t) rlim.rlim_cur;
    
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"files_n=%ld\n\n", night_cycle->files_n );
    
    night_cycle->files = 
    	night_pmalloc(night_cycle->pool, sizeof(night_connection_t*) * night_cycle->files_n );
	
	if (night_cycle->files == NULL) 
	{
		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(error_log_fd,"night_pmalloc failed for night_cycle->files\n\n");
		
		return NIGHT_ERROR;
	}
	
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
	dprintf(trace_file_fd,"night_cycle->files is completed\n\n" );
	
    
	night_cycle->connections =
        night_pmalloc(night_cycle->pool, sizeof(night_connection_t) * night_cycle->connection_n);
    if (night_cycle->connections == NULL ) 
    {
    	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd,"night_pmalloc failed for night_cycle->connections\n\n");
    	
        return NIGHT_ERROR;
    }
    
    c = night_cycle->connections;
    
	night_cycle->read_events 
		= night_pmalloc(night_cycle->pool, sizeof(night_event_t) * night_cycle->connection_n);
    if (night_cycle->read_events == NULL) 
    {
		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd,"night_pmalloc failed for night_cycle->read_events\n\n");
    	
        return NIGHT_ERROR;
    }

    rev = night_cycle->read_events;
    for (i = 0; i < night_cycle->connection_n; i++) 
    {
        rev[i].closed = 1;
        rev[i].instance = 1;
    }

    night_cycle->write_events 
    	= night_pmalloc(night_cycle->pool, sizeof(night_event_t) * night_cycle->connection_n);
    if (night_cycle->write_events == NULL) 
    {
		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd,"night_pmalloc failed for night_cycle->write_events \n\n");
    	
        return NIGHT_ERROR;
    }

    wev = night_cycle->write_events;
    for (i = 0; i < night_cycle->connection_n; i++) 
    {
        wev[i].closed = 1;
    }
    
    i = night_cycle->connection_n;
    next = NULL;
    
	do
	{
        i--;

        c[i].next = next;
        c[i].read = &night_cycle->read_events[i];
        c[i].write = &night_cycle->write_events[i];
        c[i].fd = -1;

        next = &c[i];
        
    } while (i);
    
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"night_cycle->connections init is completed\n\n" );
    
	night_cycle->free_connections = next;
	night_cycle->free_connection_n = night_cycle->connection_n;
		 
	/* for each listening socket */
	ls = night_cycle->listening.elts;	
	for (i = 0; i < night_cycle->listening.nelts; i++) 
	{
		if (ls[i].worker != night_worker) 
		{
			dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
			dprintf(trace_file_fd,"ls[i].worker(%d) != night_worker(%d)\n\n" , ls[i].worker, night_worker);
			
            continue;
        }
        
        c = night_get_connection(ls[i].fd);
		if (c == NULL) 
		{
            return NIGHT_ERROR;
        }
        
		c->type = ls[i].type;
        c->listening = &ls[i];
        ls[i].connection = c;

        rev = c->read;
        rev->accept = 1;
        
        rev->handler = (c->type == SOCK_STREAM) ? night_event_accept : night_event_recvmsg;
        
		if (night_add_event(rev, NIGHT_READ_EVENT, EPOLLET) == NIGHT_ERROR) 
		{
			return NIGHT_ERROR;
		}
	}	 
    
    return NIGHT_OK;
}
