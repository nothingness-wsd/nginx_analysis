#include "night_core.h"
#include "night_process_events.h"
#include "night_event_actions.h"
#include "night_event_timer.h"
#include "night_event.h"
#include "night_event_posted.h"

void
night_process_events_and_timers()
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"function:\t" "night_process_events_and_timers\n\n");
    
	uint64_t  	flags;
    int64_t  	timer, delta;
    int 		rc;

	timer = night_event_find_timer();
	flags = NIGHT_UPDATE_TIME;
	
	if (!night_queue_empty(&night_posted_next_events)) 
	{
        dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        dprintf(trace_file_fd,"night_posted_next_events not empty\n\n");
        
        night_event_move_posted_next();
        timer = 0;
    }
   
    delta = night_current_msec;
    
	night_process_events(timer, flags);
    
    delta = night_current_msec - delta;
    
	//night_event_process_posted(&night_posted_accept_events);
   

    //night_event_expire_timers();

    //night_event_process_posted(&night_posted_events);
}

int 
night_process_events(int64_t timer, uint32_t flags)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"function:\t" "night_process_events\n\n");
    
    int rc;
    
    rc = night_event_actions.process_events(timer, flags);
    
    return rc;
}
