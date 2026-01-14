#include "night_core.h"
#include "night_array.h"
#include "night_pool.h"

int
night_array_init(night_array_t *array, night_pool_t *pool, int n, size_t size)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_array_init\n\n");

    array->nelts = 0;
    array->size = size;
    array->nalloc = n;
    array->pool = pool;
 
    array->elts = night_pmalloc(pool, n * size);
    if (array->elts == NULL) 
    {
		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd,"night_pmalloc failed\n\n");
    
        return NIGHT_ERROR;
    }
 
    return NIGHT_OK;
}

night_array_t*
night_array_create(night_pool_t *pool, int n, size_t size)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_array_create\n\n");
    
    night_array_t *array;
    
    array = night_pmalloc( pool, sizeof(night_array_t) );
    if( array == NULL )
    {
		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd,"night_pmalloc failed\n\n");
    	
        return NULL;
    }
    
    if (night_array_init(array, pool, n, size) != NIGHT_OK)
    {
		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd,"night_array_init failed\n\n");
    	
        return NULL;
    }
    
    return array;
}

void*
night_array_push(night_array_t *array)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_array_push\n\n");
    
    night_pool_t 	*pool;
    size_t 			size;
    void			*new;
    void			*elt; 
    int				n;
    int				ex_n;
    
    n = array->nelts;
    if (array->nelts == array->nalloc)
    {
    	if (n < 100)
    	{
    		ex_n = 2 * n;
    	}
    	else
    	{
    		ex_n = n + 100;
    	}
    	
        size = array->size * ex_n;
        pool = array->pool;
        
        new = night_pmalloc(pool, size);
        if( new == NULL)
        {
       		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    		dprintf(error_log_fd,"night_pmalloc failed\n\n");
    	
            return NULL;
        }
        
        memcpy(new, array->elts, size);
        
        array->elts = new;
        array->nalloc = ex_n;
    }
    
    elt = (char*) array->elts + array->size * array->nelts;
    array->nelts++;
    
    return elt;
}
