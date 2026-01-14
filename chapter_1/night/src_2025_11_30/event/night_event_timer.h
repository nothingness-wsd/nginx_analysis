#ifndef _NIGHT_EVENT_TIMER_H_
#define _NIGHT_EVENT_TIMER_H_

#define NIGHT_TIMER_LAZY_DELAY  600
#define NIGHT_TIMER_INFINITE 	(-1)

int
night_event_timer_init();

void
night_event_add_timer(night_event_t *ev, int64_t timer);

void
night_event_del_timer(night_event_t *ev);

int64_t
night_event_find_timer(void);

#endif	/* _NIGHT_EVENT_TIMER_H_ */
