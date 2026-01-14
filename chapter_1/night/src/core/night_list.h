#ifndef _NIGHT_LIST_H_
#define _NIGHT_LIST_H_

typedef struct night_list_s 		night_list_t;
typedef struct night_list_part_s 	night_list_part_t;

struct night_list_part_s 
{
    void				*elts;
    size_t				nelts;
    night_list_part_t	*next;
};

struct night_list_s
{
    night_list_part_t	*last;
    night_list_part_t	part;
    size_t				size;
    size_t				nalloc;
    night_pool_t		*pool;
};

int
night_list_init(night_list_t *list, night_pool_t *pool, size_t n, size_t size);

#endif /* _NIGHT_LIST_H_ */
