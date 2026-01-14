#ifndef _NIGHT_EVENT_H_
#define _NIGHT_EVENT_H_

#include "night_rbtree.h"
#include "night_queue.h"

#define NIGHT_INVALID_INDEX (-1)

#define NIGHT_CLOSE_EVENT 	(1)

#define NIGHT_UPDATE_TIME 	(1)
#define NIGHT_POST_EVENTS 	(2)

struct night_event_s 
{
	void 					*data;
	night_event_handler_pt 	handler;
	int 					index;
	unsigned 				write:1;
	unsigned 				accept:1;
	unsigned 				channel:1;
	unsigned 				pending_eof:1;
	unsigned 				ready:1;
	unsigned 				eof:1;
	unsigned 				error:1;
	unsigned 				timedout:1;
	unsigned 				timer_set:1;
	unsigned 				active:1;
	unsigned 				instance:1;
	unsigned 				closed:1;
	unsigned 				posted:1;
	unsigned				cancelable:1;
	
	int 					available;
	
	night_rbtree_node_t 	timer;
	
	// the posted queue
	night_queue_t 			queue;
};

int
night_add_event(night_event_t *ev, int event, uint32_t flags);

int
night_del_event(night_event_t *ev, int event, uint32_t flags);

int
night_del_conn(night_connection_t *c, uint32_t flags);

int
night_handle_read_event(night_event_t *rev);

#endif /* _NIGHT_EVENT_H_ */
