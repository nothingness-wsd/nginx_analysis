#include "night_core.h"
#include "night_event.h"
#include "night_event_actions.h"

int
night_add_event(night_event_t *ev, int event, uint32_t flags)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"function:  night_add_event\n\n");
    
    int rc;
    
	rc = night_event_actions.add_event(ev, event, flags);
	
	return rc;
}

int
night_del_event(night_event_t *ev, int event, uint32_t flags)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"function:\t" "night_del_event\n\n");
    
    int rc;
    
	rc = night_event_actions.del_event(ev, event, flags);
	
	return rc;
}


int
night_del_conn(night_connection_t *c, uint32_t flags)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"function:\t" "night_del_conn\n\n");
    
    int rc;
    
	rc = night_event_actions.del_conn(c, flags);
	
	return rc;
}

int
night_handle_read_event(night_event_t *rev)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"function:\t" "night_handle_read_event\n\n");
    
	if (!rev->active && !rev->ready) 
	{
		dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
		dprintf(trace_file_fd,"night_add_event\n\n");
		
		if (night_add_event(rev, NIGHT_READ_EVENT, EPOLLET) == NIGHT_ERROR)
		{
			dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
			dprintf(error_log_fd,"night_add_event failed\n\n");
				
			return NIGHT_ERROR;
		}
	}
    
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"function night_handle_read_event:\treturn NIGHT_OK;\n\n");
    
    return NIGHT_OK;
}
