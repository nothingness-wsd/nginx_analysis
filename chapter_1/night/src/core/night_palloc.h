#ifndef _NIGHT_PALLOC_H_
#define _NIGHT_PALLOC_H_

#define NIGHT_POOL_ALIGNMENT	16

typedef struct night_pool_s 		night_pool_t;
typedef struct night_pool_data_s 	night_pool_data_t;
typedef struct night_pool_large_s 	night_pool_large_t;
typedef struct night_pool_cleanup_s night_pool_cleanup_t;

typedef void (*night_pool_cleanup_pt)(void *data);

struct night_pool_cleanup_s 
{
    night_pool_cleanup_pt   handler;
    void                 	*data;
    night_pool_cleanup_t   	*next;
};

struct night_pool_large_s 
{
    night_pool_large_t     	*next;
    void                 	*alloc;
};

struct night_pool_data_s
{
    char				*last;
    char				*end;
    night_pool_t		*next;
    uint32_t			failed;
};

struct night_pool_s 
{
    night_pool_data_t		d;
    size_t					max;
    night_pool_t			*current;
    night_pool_large_t     	*large;
    night_pool_cleanup_t   	*cleanup;
    //night_chain_t			*chain;
};

night_pool_t*
night_create_pool(size_t size);

void *
night_pcalloc(night_pool_t *pool, size_t size);

void*
night_palloc(night_pool_t *pool, size_t size);

void
night_destroy_pool(night_pool_t *pool);

void *
night_pnalloc(night_pool_t *pool, size_t size);


#endif /* _NIGHT_PALLOC_H_ */
