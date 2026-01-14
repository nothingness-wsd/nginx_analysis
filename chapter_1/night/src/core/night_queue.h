#ifndef _NIGHT_QUEUE_H_
#define _NIGHT_QUEUE_H_

typedef struct night_queue_s	night_queue_t;

struct night_queue_s 
{
    night_queue_t  *prev;
    night_queue_t  *next;
};


#define night_queue_init(q)                                                     \
		(q)->prev = q;                                                            \
		(q)->next = q
    
#endif /* _NIGHT_QUEUE_H_ */
