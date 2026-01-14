#include "night_core.h"
#include "night_pool.h"
#include "night_string.h"

night_pool_t*
night_create_pool(size_t size)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_create_pool\n\n");
    
    night_pool_t* p;
    
    p = malloc(size);
    if(p == NULL)
    {
    	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(error_log_fd,"malloc failed while night_create_pool\n\n");
        
        return NULL;
    }
    
    p->data.last = (char*) p + sizeof(night_pool_t);
    p->data.end = (char*) p + size;
    p->data.next = NULL;
    p->data.failed = 0;
    
    size = size - sizeof(night_pool_t);
    
    p->limit = NIGHT_DEFAULT_POOL_LIMIT;
    
    p->current = (night_pool_data_t*) p;
    p->large = NULL;
    p->chain = NULL;
    
    return p;
}

void*
night_pmalloc(night_pool_t *pool, size_t size)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_pmalloc\n\n");
    
    void *p;
    
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"size=%ld\n", size);
    
    size = night_align(size, NIGHT_ALIGN);
    
    dprintf(trace_file_fd,"size after night_align=%ld\n\n", size);
    
    if (size < pool->limit) 
    {
		dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd,"p = night_pmalloc_small(pool, size);\n\n");
    	
        p = night_pmalloc_small(pool, size);
    }
    else
    {
    	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd,"p = night_pmalloc_large(pool, size);\n\n");
    	
        p = night_pmalloc_large(pool, size);
    }
    
    if (p) 
    {
        night_memzero(p, size);
    }

    return p;
}

void*
night_pmalloc_small(night_pool_t* pool, size_t size)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_pmalloc_small\n\n");
    
    night_pool_data_t 	*data;
    char 				*m;
    
    data = pool->current;
		
    do
    {
        m = data->last;
        
        if ( (size_t) (data->end - m) >= size) 
        {    
            data->last = m + size;
            
            return (void*) m;
        }
        
        data = data->next;
        
    }while(data);
            
    return night_pmalloc_block(pool, size);
}

void*
night_pmalloc_block(night_pool_t* pool, size_t size)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_pmalloc_block\n\n");
    
    night_pool_data_t	*data;
    char 				*m;
    char 				*r;
    night_pool_data_t 	*p;
    
    m = malloc(NIGHT_DEFAULT_POOL_SIZE);
    
    data = (night_pool_data_t*) m;
    
    data->last = m + sizeof(night_pool_data_t);
    data->end = m + NIGHT_DEFAULT_POOL_SIZE;
    data->next = NULL;
    data->failed = 0;
    
    r = data->last;
    
    data->last = data->last + size;
    
    for	(p = pool->current; p->next; p = p->next)
    {
        if (p->failed++ > 4) 
        {
            pool->current = p->next;
        }
    }
    
    p->next = data;
    
    return r;  
}

void*
night_pmalloc_large(night_pool_t *pool, size_t size)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_pmalloc_large\n\n");

    night_pool_large_t 	*l;
    char				*p;
    
    l = night_pmalloc(pool, sizeof(night_pool_large_t));
    
	p = malloc(size);
    
    l->malloc = p;
    
    l->next = pool->large;
    
    pool->large = l;
    
    return p;
}

void
night_destroy_pool(night_pool_t *pool)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_destroy_pool\n\n");
    
    night_pool_large_t 	*large;
    night_pool_data_t 	*data;
    night_pool_data_t 	*data_next;
    
    large = pool->large;
    while (large)
    {
    	if (large->malloc)
    	{
    		free(large->malloc);
    		large->malloc = NULL;
    	}
    	large = large->next;
    }
    
    data = pool->data.next;
    while (data)
    {
    	data_next = data->next;
    	
    	free(data);
    	
    	data = data_next;
    }
    
	free(pool);
    pool = NULL;
}

int
night_plfree(night_pool_t *pool, void *p)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_plfree\n\n");

    night_pool_large_t  *l;

    for (l = pool->large; l; l = l->next) 
    {
        if (p == l->malloc) 
        {
        	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "free:%p\n\n", l->malloc);
        	
            free(l->malloc);
            l->malloc = NULL;

            return NIGHT_OK;
        }
    }
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "night_plfree:free large block memory %p failed\n\n", p);
        	
    return NIGHT_DECLINED;
}
