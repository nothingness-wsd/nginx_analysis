#include "night_core.h"

int
night_array_init(night_array_t *array, night_pool_t *pool, size_t n, size_t size)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "%s\n\n", __func__);

    array->nelts = 0;
    array->size = size;
    array->nalloc = n;
    array->pool = pool;

    array->elts = night_palloc(pool, n * size);
    if (array->elts == NULL) 
    {
        return NIGHT_ERROR;
    }

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function %s:\t" "return NIGHT_OK\n\n", __func__);
	
    return NIGHT_OK;
}

night_array_t *
night_array_create(night_pool_t *p, size_t n, size_t size)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "%s\n\n", __func__);
	
    night_array_t *a;

    a = night_palloc(p, sizeof(night_array_t));
    if (a == NULL) 
    {
        return NULL;
    }

    if (night_array_init(a, p, n, size) != NIGHT_OK) 
    {
        return NULL;
    }

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function %s:\t" "return\n\n", __func__);
	
    return a;
}

void *
night_array_push(night_array_t *a)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function:\t" "%s\n\n", __func__);
	
    void			*elt, *new;
    size_t			size;
    night_pool_t  	*p;

    if (a->nelts == a->nalloc) 
    {
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "数组已满\n\n");

        size = a->size * a->nalloc;

        p = a->pool;

        if ((char *) a->elts + size == p->d.last
            && p->d.last + a->size <= p->d.end)
        {
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(trace_file_fd, "连续地址直接扩容\n\n");

            p->d.last += a->size;
            a->nalloc++;

        } 
        else 
        {
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(trace_file_fd, "重新分配内存扩容\n\n");
			
            new = night_palloc(p, 2 * size);
            if (new == NULL) 
            {
                return NULL;
            }

            memcpy(new, a->elts, size);
            a->elts = new;
            a->nalloc *= 2;
        }
    }

    elt = (char *) a->elts + a->size * a->nelts;
    a->nelts++;

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "元素个数:%ld\n" "function %s:\t" "return\n\n", a->nelts, __func__);
	
    return elt;
}
