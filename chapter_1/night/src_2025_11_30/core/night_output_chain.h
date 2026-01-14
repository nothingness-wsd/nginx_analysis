#ifndef _NIGHT_OUTPUT_CHAIN_
#define _NIGHT_OUTPUT_CHAIN_

#define NIGHT_NONE			1

typedef int (*night_output_chain_filter_pt)(void *ctx, night_chain_t *in);

typedef struct night_output_chain_ctx_s  	night_output_chain_ctx_t; 
typedef struct night_bufs_s 				night_bufs_t;

struct night_bufs_s
{
    int    			num;
    size_t       	size;
};

struct night_output_chain_ctx_s 
{
	night_buf_t						*buf;
	night_chain_t					*in;
	night_chain_t					*free;
	night_chain_t                 	*busy;
	
	night_bufs_t					bufs;
		
	night_pool_t					*pool;
	
	night_output_chain_filter_pt   	output_filter;
	
	void                        	*filter_ctx;
	
	void*                			tag;
};

int
night_output_chain(night_output_chain_ctx_t *ctx, night_chain_t *in);

int
night_output_chain_add_copy(night_pool_t *pool, night_chain_t **chain, night_chain_t *in);

int
night_output_chain_align_file_buf(night_output_chain_ctx_t *ctx, off_t bsize);

int
night_output_chain_get_buf(night_output_chain_ctx_t *ctx, off_t bsize);

int
night_output_chain_copy_buf(night_output_chain_ctx_t *ctx);

#endif /* _NIGHT_OUTPUT_CHAIN_ */
