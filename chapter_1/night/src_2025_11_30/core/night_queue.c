#include "night_core.h"
#include "night_queue.h"

void
night_queue_sort(night_queue_t *queue, int (*cmp)(const night_queue_t *one, const night_queue_t *two))
{
    
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_queue_sort\n\n");
    
    night_queue_t *q,*prev,*next;
    
    q = night_queue_head(queue);
    
    if (q == night_queue_last(queue)) 
    {
        return;
    }
    
    for (q = night_queue_next(q); q != night_queue_sentinel(queue); q = next) 
    {    
        prev = night_queue_prev(q);
        next = night_queue_next(q);
        
        night_queue_remove(q);
        
        do
        {
            if (cmp(prev, q) <= 0) 
            {
                break;
            }

            prev = night_queue_prev(prev);

        }while(prev != night_queue_sentinel(queue));

        night_queue_insert_after(prev, q);
    }
}

night_queue_t*
night_queue_middle(night_queue_t *queue)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function:\t" "night_queue_middle\n\n");
	
	night_queue_t	*middle;
    night_queue_t	*next;
    
    middle = night_queue_head(queue);
    if (middle == night_queue_last(queue)) 
    {
        return middle;
    }
    
    next = night_queue_head(queue);
    
    for ( ; ; ) 
    {
		middle = night_queue_next(middle);

        next = night_queue_next(next);

        if (next == night_queue_last(queue)) 
        {
            return middle;
        }

        next = night_queue_next(next);

        if (next == night_queue_last(queue)) 
        {
            return middle;
        }
    }
}
