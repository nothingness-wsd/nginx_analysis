#include "night_core.h"
#include "night_list.h"
#include "night_pool.h"

int
night_list_init(night_list_t *list, night_pool_t *pool, int n, size_t size)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
	dprintf(trace_file_fd, "function:\t" "night_list_init\n\n");
	
    list->part.elts = night_pmalloc(pool, n * size);
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

    return NIGHT_OK;
}


void*
night_list_push(night_list_t *list)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
	dprintf(trace_file_fd, "function:\t" "night_list_push\n\n");
	
    void             	*elt;
    night_list_part_t  	*last;

    last = list->last;

    if (last->nelts == list->nalloc) 
    {
        // the last part is full, allocate a new list part
        last = night_pmalloc(list->pool, sizeof(night_list_part_t));
        if (last == NULL) 
        {
            return NULL;
        }

        last->elts = night_pmalloc(list->pool, list->nalloc * list->size);
        if (last->elts == NULL) 
        {
            return NULL;
        }

        last->nelts = 0;
        last->next = NULL;

        list->last->next = last;
        list->last = last;
    }

    elt = (char *) last->elts + list->size * last->nelts;
    last->nelts++;

    return elt;
}
