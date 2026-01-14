#include "night_core.h"
#include "night_event_posted.h"
#include "night_event.h"

night_queue_t night_posted_next_events;
night_queue_t night_posted_accept_events;
night_queue_t night_posted_events;

int
night_add_post_event(night_queue_t *h, night_event_t *e)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_add_post_event\n\n");
    
    if (!e->posted)
    {
    	e->posted = 1; 
    	night_queue_insert_tail(h, &(e->queue));
    }
    
    return NIGHT_OK;
}

int
night_delete_posted_event(night_event_t *e)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"function:\t" "night_delete_posted_event\n\n");
    
    e->posted = 0;
    night_queue_remove(&e->queue);   
    
    return NIGHT_OK;
}

int
night_event_process_posted(night_queue_t *posted)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"function:\t" "night_event_process_posted\n\n");
    
	night_queue_t  	*q;
    night_event_t 	*ev;
    
    while (!night_queue_empty(posted))
    {
        q = night_queue_head(posted);
        
        ev = night_queue_data(q, night_event_t, queue);

        night_delete_posted_event(ev);

        ev->handler(ev);
    }
    
    return NIGHT_OK;
}

int
night_event_move_posted_next()
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"function:\t" "night_event_move_posted_next\n\n");
    
	night_queue_t  *q;
    night_event_t  *ev;
    
    for (q = night_queue_head(&night_posted_next_events);
    	 q != night_queue_sentinel(&night_posted_next_events);
    	 q = night_queue_next(q))
    {
		ev = night_queue_data(q, night_event_t, queue);
    	
		ev->ready = 1;
        ev->available = -1;
    }
    
	night_queue_add(&night_posted_events, &night_posted_next_events);
    night_queue_init(&night_posted_next_events);
    
    return NIGHT_OK;
}


