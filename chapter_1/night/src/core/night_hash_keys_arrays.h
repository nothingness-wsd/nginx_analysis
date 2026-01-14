#ifndef _NIGHT_HASH_KEYS_ARRAYS_H_
#define _NIGHT_HASH_KEYS_ARRAYS_H_

typedef struct night_hash_keys_arrays_s night_hash_keys_arrays_t;

struct night_hash_keys_arrays_s
{
    night_pool_t	*pool;
    night_pool_t	*temp_pool;
    
	size_t        	hsize;

    night_array_t	keys;
    night_array_t	*keys_hash;

    night_array_t	dns_wc_head;
    night_array_t	*dns_wc_head_hash;

    night_array_t	dns_wc_tail;
    night_array_t	*dns_wc_tail_hash;
};
#endif /* _NIGHT_HASH_KEYS_ARRAYS_H_ */
