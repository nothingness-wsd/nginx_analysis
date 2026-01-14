#include "night_core.h"

int
night_list_init(night_list_t *list, night_pool_t *pool, size_t n, size_t size)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "%s\n\n", __func__);
	
    list->part.elts = night_palloc(pool, n * size);
    if (list->part.elts == NULL) 
    {
        return NIGHT_ERROR;
    }

    list->part.nelts = 0;
    list->part.next = NULL;
    list->last = &list->part;
    list->size = size;
    list->nalloc = n;
    list->pool = pool;

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function %s:\t" "return NIGHT_OK\n\n", __func__);
	
    return NIGHT_OK;
}
