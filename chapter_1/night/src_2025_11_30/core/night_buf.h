#ifndef _NIGHT_BUF_H_
#define _NIGHT_BUF_H_

#include "night_pool.h"

#define night_buf_size(b)													\
    	((b->in_file) ? (off_t) ((b)->file_last  - (b)->file_pos):			\
                            	((b)->last - (b)->pos))
                            	
                           	

#define NIGHT_CHAIN_ERROR     (night_chain_t *) NIGHT_ERROR          

#define night_pmalloc_buf(pool) night_pmalloc(pool, sizeof(night_buf_t))

#define night_free_chain(pool, cl)											\
    	(cl)->next = (pool)->chain;											\
    	(pool)->chain = (cl)
                            
struct night_buf_s
{
    char				*start;
    char				*pos;
    char				*last;
    char				*end;
    unsigned 			temporary:1;
    
    night_file_t		*file;
    
	off_t            	file_pos;
    off_t            	file_last;
    
    unsigned         	last_buf:1;
	unsigned         	recycled:1;
    unsigned         	flush:1;
    unsigned         	sync:1;
	unsigned         	last_in_chain:1;
	unsigned         	in_file:1;
	
	void*    			tag;
};

struct night_chain_s 
{
    night_buf_t 	*buf;
    night_chain_t 	*next;
};

night_buf_t*
night_create_temp_buf(night_pool_t *pool, size_t size);

night_chain_t *
night_pmalloc_chain_link(night_pool_t *pool);

night_chain_t *
night_chain_update_sent(night_chain_t *in, off_t sent);

void
night_chain_update_chains(night_pool_t *p, night_chain_t **free, night_chain_t **busy, night_chain_t **out, void* tag);

#endif /* _NIGHT_BUF_H_ */
