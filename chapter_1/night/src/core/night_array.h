#ifndef _NIGHT_ARRAY_H_
#define _NIGHT_ARRAY_H_

typedef struct night_array_s night_array_t;

struct night_array_s
{
    void			*elts;
    size_t			nelts;
    size_t       	size;
    size_t   		nalloc;
    night_pool_t  	*pool;
};

int
night_array_init(night_array_t *array, night_pool_t *pool, size_t n, size_t size);

night_array_t *
night_array_create(night_pool_t *p, size_t n, size_t size);

void *
night_array_push(night_array_t *a);

#endif /* _NIGHT_ARRAY_H_ */
