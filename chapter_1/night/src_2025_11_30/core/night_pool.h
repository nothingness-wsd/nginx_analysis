#ifndef _NIGHT_POOL_H_
#define _NIGHT_POOL_H_

#define NIGHT_DEFAULT_POOL_SIZE 	(4 * 4 * 1024)
#define NIGHT_DEFAULT_POOL_LIMIT 	(4 * 1024)

typedef struct night_pool_data_s 	night_pool_data_t;
typedef struct night_pool_large_s 	night_pool_large_t;


struct night_pool_data_s
{
    char				*last;
    char				*end;
    night_pool_data_t 	*next;
    uint8_t 			failed;
};

struct night_pool_large_s
{
    night_pool_large_t 	*next;
    void				*malloc;
};

struct night_pool_s
{
    night_pool_data_t 	data;
    size_t 				limit;
    night_pool_data_t	*current;
    night_pool_large_t	*large;
    
    night_chain_t		*chain;
};

night_pool_t*
night_create_pool(size_t size);

void*
night_pmalloc(night_pool_t *pool, size_t size);

void*
night_pmalloc_small(night_pool_t* pool, size_t size);

void*
night_pmalloc_block(night_pool_t* pool, size_t size);

void*
night_pmalloc_large(night_pool_t *pool, size_t size);

void
night_destroy_pool(night_pool_t *pool);

int
night_plfree(night_pool_t *pool, void *p);

#endif /* _NIGHT_POOL_H_ */
