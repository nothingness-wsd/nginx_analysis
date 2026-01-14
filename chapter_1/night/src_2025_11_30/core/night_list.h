#ifndef _NIGHT_LIST_H_
#define _NIGHT_LIST_H_

typedef struct night_list_s 		night_list_t;
typedef struct night_list_part_s 	night_list_part_t;

struct night_list_part_s 
{
    void							*elts;
    uint32_t	        			nelts;
    night_list_part_t				*next;
};

struct night_list_s
{
    night_list_part_t  				*last;
    night_list_part_t   			part;
    size_t            				size;
    uint32_t		        		nalloc;
    night_pool_t       				*pool;
};

int
night_list_init(night_list_t *list, night_pool_t *pool, int n, size_t size);

void*
night_list_push(night_list_t *list);

#endif /* _NIGHT_LIST_H_ */
