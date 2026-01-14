#ifndef _NIGHT_QUEUE_H_
#define _NIGHT_QUEUE_H_

#define night_queue_init(q) \
        (q)->prev = q; 		\
        (q)->next = q
        
#define night_queue_insert_tail(h, x) \
        (x)->prev = (h)->prev;        \
        (x)->prev->next = x;          \
        (x)->next = h;                \
        (h)->prev = x 

#define night_queue_empty(h) \
        (h == (h)->prev)
        
#define night_queue_head(h) \
        (h)->next     
        
#define night_queue_last(h) \
        (h)->prev       
        
#define night_queue_sentinel(h) \
        (h)            
        
#define night_queue_next(q) \
        (q)->next

#define night_queue_prev(q) \
        (q)->prev      
        
#define night_queue_remove(x)  \
        (x)->next->prev = (x)->prev;  \
        (x)->prev->next = (x)->next 
        
#define night_queue_insert_head(h, x) \
        (x)->next = (h)->next;        \
        (x)->next->prev = x;          \
        (x)->prev = h;                \
        (h)->next = x         

#define night_queue_insert_after night_queue_insert_head

#define night_queue_split(h, q, n)	\
        (n)->prev = (h)->prev;		\
        (n)->prev->next = n;		\
        (n)->next = q;				\
        (h)->prev = (q)->prev;		\
        (h)->prev->next = h;		\
        (q)->prev = n

#define night_queue_add(h, n)			\
        (h)->prev->next = (n)->next;	\
        (n)->next->prev = (h)->prev;	\
        (h)->prev = (n)->prev;			\
        (h)->prev->next = h
 
#define night_queue_data(q, type, link)	\
    (type *) ((char *) q - offsetof(type, link))
                            

struct night_queue_s
{
    night_queue_t *prev;
    night_queue_t *next;
};

void
night_queue_sort(night_queue_t *queue, int (*cmp)(const night_queue_t *one, const night_queue_t *two));

night_queue_t*
night_queue_middle(night_queue_t *queue);

#endif /* _NIGHT_QUEUE_H_ */
