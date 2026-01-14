#include "night_core.h"
#include "night_epoll_module.h"
#include "night_module.h"
#include "night_event_module_ctx.h"
#include "night_cycle.h"
#include "night_pool.h"
#include "night_conf.h"
#include "night_event.h"
#include "night_connection.h"
#include "night_event_timer.h"
#include "night_time.h"
#include "night_event_posted.h"
#include "night_os_io.h"

	
night_command_t night_epoll_commands[] = 
{
	{
		night_string("epoll_events"),
		NIGHT_EVENT_CONF,
		offsetof(night_epoll_conf_t, events),
		night_conf_set_num
	},
    night_null_command
};


night_event_module_ctx_t night_epoll_module_ctx = 
{
    night_epoll_create_conf,
    night_epoll_init_conf,
	{
		night_epoll_init,
		night_epoll_add_event,
		night_epoll_del_event,
		night_epoll_process_events,
		night_epoll_del_conn
//		night_epoll_add_conn,
//		night_epoll_done
	}
};

night_module_t night_epoll_module = 
{
    "night_epoll_module",
    NIGHT_MODULE_UNSET_INDEX,
    NIGHT_MODULE_UNSET_INDEX,
    NIGHT_EVENT_MODULE,
    night_epoll_commands,
    &night_epoll_module_ctx,
    NULL,
    NULL,
    NULL
};

int 						ep = -1;
static struct epoll_event 	*event_list = NULL;
static int 					max_events = 0;
night_os_io_t				night_io;

void*
night_epoll_create_conf()
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_epoll_create_conf\n\n");
    
    night_epoll_conf_t *epcf;

    epcf = night_pmalloc(night_cycle->pool, sizeof(night_epoll_conf_t));
    if (epcf == NULL) 
    {
		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd,"night_pmalloc failed \n\n");
    	
        return NULL;
    }

    epcf->events = NIGHT_CONF_INT_UNSET;
    //epcf->aio_requests = NGX_CONF_UNSET;

    return epcf;
}

int
night_epoll_init_conf(void *conf)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_epoll_init_conf\n\n");
    
    night_epoll_conf_t *epcf = conf;
    
    //epcf = night_get_event_conf(night_cycle->conf_ctx, night_epoll_module);
    
    if(epcf->events < 1)
    {
    	epcf->events = 512;
    }

	return NIGHT_OK;
}  

int
night_epoll_init()
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_epoll_init\n\n");
    
    night_epoll_conf_t *epcf;
    
	if (ep == -1) 
	{
		ep = epoll_create(1024);

        if (ep == -1) 
        {
        	dprintf(error_log_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(error_log_fd, "epoll_create() failed\n\n");
            
            return NIGHT_ERROR;
        }
    }
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "epoll_create success:\nep = %d\n\n", ep);
    
    epcf = night_get_event_conf(night_cycle->conf_ctx, night_epoll_module);
    
	if (max_events < epcf->events) 
	{
        if (event_list) 
        {
            free(event_list);
            event_list = NULL;
        }

        event_list = malloc(sizeof(struct epoll_event) * epcf->events);
        if (event_list == NULL) 
        {
        	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(error_log_fd,"malloc for event_list failed\n\n");
        	
            return NIGHT_ERROR;
        }
    }
    
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"event_list = %p\n\n", event_list);
	
    max_events = epcf->events;
    
    night_io = night_os_io;
    
    night_event_actions = night_epoll_module_ctx.actions;
    
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"night_event_actions = night_epoll_module_ctx.actions\n\n");
    
    return NIGHT_OK;
}

int
night_epoll_add_event(night_event_t *ev, int event, uint32_t flags)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_epoll_add_event\n\n");
    
    night_event_t 			*e;
    uint32_t 				events;
    uint32_t 				prev;
    night_connection_t 		*c;
    int 					op;
    struct epoll_event 		ee;
    int 					rc;
    
    c = ev->data;
    
    events = (uint32_t)event;

	if (event == NIGHT_READ_EVENT)
	{
		e = c->write;
		prev = EPOLLOUT;
	}
	else
	{
		e = c->read;
        prev = EPOLLIN;
	}
	
	if (e->active) 
	{
		op = EPOLL_CTL_MOD;
        events |= prev;
	}
	else
	{
		op = EPOLL_CTL_ADD;
	}
	
	ee.events = events | (uint32_t) flags;
    ee.data.ptr = (void *) ((uintptr_t) c | ev->instance);
    
    rc = epoll_ctl(ep, op, c->fd, &ee);
	if (rc == -1) 
	{
		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd,"epoll_ctl(%d, %d) failed\n\n", op, c->fd);
    	
        return NIGHT_ERROR;
    }
    
    ev->active = 1;
    
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"night_epoll_add_event completed\n\n");
    
    return NIGHT_OK;
}

int
night_epoll_del_event(night_event_t *ev, int event, uint32_t flags)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_epoll_del_event\n\n");
    
    night_event_t 			*e;
	night_connection_t 		*c;
	int 					prev;
	int 					op;
	struct epoll_event 		ee;
	
	if (flags & NIGHT_CLOSE_EVENT) 
	{
        ev->active = 0;
        return NIGHT_OK;
    }
    
	c = ev->data;
	
	if (event == NIGHT_READ_EVENT)
	{
		e = c->write;
		prev = EPOLLOUT;
	}
	else
	{
		e = c->read;
		prev = EPOLLIN|EPOLLRDHUP;
	}
	
	if (e->active)
	{
		op = EPOLL_CTL_MOD;
        ee.events = prev;
        ee.data.ptr = (void*) ((uintptr_t) c | ev->instance);
	}
	else
	{
		op = EPOLL_CTL_DEL;
        ee.events = 0;
        ee.data.ptr = NULL;
	}

    if (epoll_ctl(ep, op, c->fd, &ee) == -1) 
    {
    	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd,"epoll_ctl(%d, %d) failed\n\n", op, c->fd);
    	
        return NIGHT_ERROR;
    }

    ev->active = 0;

    return NIGHT_OK;
}

int 
night_epoll_process_events(int64_t timer, uint32_t flags)
{
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "function:\t" "night_epoll_process_events\n\n");
    
    int 					events_n;
    int 					err;
    int 					i;
    night_connection_t 		*c;
    int 					instance;
	night_event_t 			*rev, *wev;
	uint32_t 				revents;
	night_queue_t 			*queue;
    
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"event_list = %p\n", event_list);
    dprintf(trace_file_fd,"max_events = %d\n", max_events);
    dprintf(trace_file_fd, "timer=%ld\n\n", timer);
    
    events_n = epoll_wait(ep, event_list, max_events, timer);
  	
  	  
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "epoll_wait\n");
    dprintf(trace_file_fd, "events_n=%d\n\n", events_n);
    
    
    err = (events_n == -1) ? errno : 0;
    
    if (err)
    {
		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(error_log_fd, "errno = %d:\n" "%s\n\n", err, strerror(err));
    }
	
	if (events_n == 0) 
	{
        if (timer != NIGHT_TIMER_INFINITE) 
        {
            return NIGHT_OK;
        }

		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(error_log_fd,"epoll_wait() returned no events without timeout\n\n");
		
        return NIGHT_ERROR;
    }
    
	if (flags & NIGHT_UPDATE_TIME) 
	{
        night_time_update();
    }
    
	for (i = 0; i < events_n; i++) 
	{
        c = event_list[i].data.ptr;
        
        instance = (uintptr_t) c & 1;
        c = (night_connection_t*)((uintptr_t) c & (uintptr_t)~1);
        
        revents = event_list[i].events;
        
		if (revents & (EPOLLERR|EPOLLHUP)) 
		{
			dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(error_log_fd,"epoll_wait() error on fd:%d revents:%d\n\n", c->fd, revents); 

            revents |= EPOLLIN|EPOLLOUT;
        }
        
        rev = c->read;     
        
        if( (revents & EPOLLIN) && rev->active ) 
        {
        	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        	dprintf(trace_file_fd,"event is EPOLLIN\n\n" );
        	
			if (c->fd == -1 || rev->instance != instance) 
			{    
				dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
				dprintf(error_log_fd,"epoll: stale event\n\n");
        	
            	continue;
        	}
        
			if (revents & EPOLLRDHUP) 
			{
                rev->pending_eof = 1;
            }
            
			rev->ready = 1;
            rev->available = -1;
            
			if (flags & NIGHT_POST_EVENTS) 
			{
                
                queue = rev->accept ? &night_posted_accept_events
                                    : &night_posted_events;

                night_add_post_event(queue, rev);

            } 
            else 
            {
                dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
                dprintf(trace_file_fd,"rev->handler(rev)\n\n" );
                
                rev->handler(rev);
            }
        }
        
        wev = c->write;
        
        if( (revents & EPOLLOUT) && wev->active ) 
        {
			dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        	dprintf(trace_file_fd,"event is EPOLLOUT\n\n" );

            if (c->fd == -1 || wev->instance != instance) 
            {
				dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
				dprintf(error_log_fd,"epoll: stale event\n\n");
				
				continue;
            }

            wev->ready = 1;

            if (flags & NIGHT_POST_EVENTS) 
            {
                night_add_post_event(&night_posted_events, wev);

            } 
            else 
            {
				wev->handler(wev);
            }
        }
	}
    
    return NIGHT_OK;
}


int
night_epoll_del_conn(night_connection_t *c, uint32_t flags)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_epoll_del_conn\n\n");
    
	int 				op;
    struct epoll_event 	ee;
    int 				rc;
    
	if (flags & NIGHT_CLOSE_EVENT) 
	{
        c->read->active = 0;
        c->write->active = 0;
        
        return NIGHT_OK;
    }
    
	op = EPOLL_CTL_DEL;
    ee.events = 0;
    ee.data.ptr = NULL;

	rc = epoll_ctl(ep, op, c->fd, &ee); 
    if (rc == -1) 
    {
    	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(error_log_fd,"epoll_ctl(%d, %d) failed\n\n", op, c->fd);
        
        return NIGHT_ERROR;
    }

    c->read->active = 0;
    c->write->active = 0;
    
    return NIGHT_OK;
}


/*
void
night_epoll_done()
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function: night_epoll_done\n\n");
    
    if(close(ep) == -1) 
    {
    	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd,"epoll close() failed\n\n");
    }

    ep = -1;

    free(event_list);

    event_list = NULL;
    max_events = 0;
}

int
night_epoll_add_conn(night_connection_t *c)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function: night_epoll_add_conn\n\n");
    
    struct epoll_event  ee;
    
	ee.events = EPOLLIN|EPOLLOUT|EPOLLET|EPOLLRDHUP;
    ee.data.ptr = (void *) ((uintptr_t) c | c->read->instance);

    if (epoll_ctl(ep, EPOLL_CTL_ADD, c->fd, &ee) == -1) 
    {
		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd,"epoll_ctl(EPOLL_CTL_ADD, %d) failed\n\n", , c->fd);
    	
        return NIGHT_ERROR;
    }

    c->read->active = 1;
    c->write->active = 1;

    return NIGHT_OK;
}
*/
