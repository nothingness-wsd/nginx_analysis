#ifndef _NIGHT_EVENT_POSTED_H_
#define _NIGHT_EVENT_POSTED_H_

extern night_queue_t night_posted_next_events;
extern night_queue_t night_posted_accept_events;
extern night_queue_t night_posted_events;

int
night_add_post_event(night_queue_t *h, night_event_t *e);

int
night_delete_posted_event(night_event_t *e);

int
night_event_process_posted(night_queue_t *posted);

int
night_event_move_posted_next();

#endif /* _NIGHT_EVENT_POSTED_H_ */
