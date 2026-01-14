#include "night_core.h"

static inline void*
night_palloc_small(night_pool_t *pool, size_t size, int align);

static void*
night_palloc_block(night_pool_t *pool, size_t size);

static void *
night_palloc_large(night_pool_t *pool, size_t size);

night_pool_t*
night_create_pool(size_t size)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function:\t" "night_create_pool\n\n");
	
    night_pool_t  *p;

    p = night_memalign(NIGHT_POOL_ALIGNMENT, size);
    if (p == NULL) 
    {
        return NULL;
    }

    p->d.last = (char *) p + sizeof(night_pool_t);
    p->d.end = (char *) p + size;
    p->d.next = NULL;
    p->d.failed = 0;

    size = size - sizeof(night_pool_t);
    p->max = (size < night_pagesize) ? size : night_pagesize;

    p->current =  p;
    //p->chain = NULL;
    p->large = NULL;
    p->cleanup = NULL;

	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function night_create_pool:\t" "retun\n\n");
	
    return p;
}

void *
night_pcalloc(night_pool_t *pool, size_t size)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "%s\n\n", __func__);
	
    void *p;

    p = night_palloc(pool, size);
    if (p) 
    {
        night_memzero(p, size);
    }

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function %s:\t" "return\n\n", __func__);
	
    return p;
}

void*
night_palloc(night_pool_t *pool, size_t size)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "%s\n\n", __func__);
	
	char*	m;
	
    if (size <= pool->max) 
    {
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "size(%ld) <= pool->max (%ld)\n\n", size, pool->max);
		dprintf(trace_file_fd, "小块分配\n" "night_palloc_small\n\n");
	
        return night_palloc_small(pool, size, 1);
    }

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "大块分配\n" "night_palloc_large\n\n");
	
	m = night_palloc_large(pool, size);
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function %s:\t" "return\n\n", __func__);
	
    return m;
}

static inline void*
night_palloc_small(night_pool_t *pool, size_t size, int align)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "%s\n\n", __func__);
	
    char      		*m;
    night_pool_t  	*p;

    p = pool->current;

    do 
    {
        m = p->d.last;

        if (align) 
        {
            m = night_align_ptr(m, NIGHT_ALIGNMENT);
        }

        if ((size_t) (p->d.end - m) >= size) 
        {
            p->d.last = m + size;

            return m;
        }

        p = p->d.next;

    } while (p);

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "分配新的块\n" "night_palloc_block\n\n");
	
    m = night_palloc_block(pool, size);
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function %s:\t" "return\n\n", __func__);
	
    return m;
}

static void *
night_palloc_block(night_pool_t *pool, size_t size)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "%s\n\n", __func__);
	
    char      		*m;
    size_t       	psize;
    night_pool_t  	*p, *new;

    psize = (size_t) (pool->d.end - (char *) pool);

    m = night_memalign(NIGHT_POOL_ALIGNMENT, psize);
    if (m == NULL) 
    {
        return NULL;
    }

    new = (night_pool_t *) m;

    new->d.end = m + psize;
    new->d.next = NULL;
    new->d.failed = 0;

    m += sizeof(night_pool_data_t);
    m = night_align_ptr(m, NIGHT_ALIGNMENT);
    new->d.last = m + size;

    for (p = pool->current; p->d.next; p = p->d.next) 
    {
        if (p->d.failed++ > 4) 
        {
        	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "p->d.faile=%d\n\n", p->d.failed);
        	
            pool->current = p->d.next;
        }
    }

    p->d.next = new;

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function %s:\t" "return\n\n", __func__);
	
    return m;
}

static void *
night_palloc_large(night_pool_t *pool, size_t size)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "%s\n\n", __func__);
	
    void              		*p;
    int         			n;
    night_pool_large_t  	*large;

    p = malloc(size);
    if (p == NULL) 
    {
        return NULL;
    }

    n = 0;

    for (large = pool->large; large; large = large->next) 
    {
        if (large->alloc == NULL) 
        {
            large->alloc = p;
            
            dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            dprintf(trace_file_fd, "复用 large 节点，n=%d\n", n);
            dprintf(trace_file_fd, "function %s:\t" "return\n\n", __func__);
            
            return p;
        }

        if (n++ > 3) 
        {
            break;
        }
    }

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "分配新的 large 节点\n\n");
            
    large = night_palloc_small(pool, sizeof(night_pool_large_t), 1);
    if (large == NULL) 
    {
        free(p);
        return NULL;
    }

    large->alloc = p;
    large->next = pool->large;
    pool->large = large;

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function %s:\t" "return\n\n", __func__);
	            
    return p;
}

void
night_destroy_pool(night_pool_t *pool)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function:\t" "%s\n\n", __func__);
	
    night_pool_t          	*p;
    night_pool_t			*n;
    night_pool_large_t    	*l;
    night_pool_cleanup_t  	*c;

    for (c = pool->cleanup; c; c = c->next) 
    {
    	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd,"cleanup\n\n");
    	
        if (c->handler) 
        {
        	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd,"run cleanup: %p\n", c);
    		dprintf(trace_file_fd,"c->handler(c->data)\n\n");
    	               
            c->handler(c->data);
        }
    }

    for (l = pool->large; l; l = l->next) 
    {
    	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd,"free large memory block\n\n");
    	
        if (l->alloc) 
        {
        	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd,"free(%p)\n\n", l->alloc);
        	
            free(l->alloc);
        }
    }

    for (p = pool, n = pool->d.next; /* void */; p = n, n = n->d.next) 
    {
		dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd,"free memory block of pool\n");
		dprintf(trace_file_fd, "free(%p)\n\n", p);
		
        free(p);

        if (n == NULL) 
        {
            break;
        }
    }
    
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function %s:\t" "return\n\n", __func__);
}

void *
night_pnalloc(night_pool_t *pool, size_t size)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "%s\n\n", __func__);

    if (size <= pool->max) 
    {
   		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "分配小块内存\n" "night_palloc_small(pool, size, 0)\n\n");
	
        return night_palloc_small(pool, size, 0);
    }


	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "分配大块内存\n" "night_palloc_large\n\n");
		
    return night_palloc_large(pool, size);
}

