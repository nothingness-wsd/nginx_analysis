#ifndef _NIGHT_EVENT_ACCEPT_H_
#define _NIGHT_EVENT_ACCEPT_H_

void
night_event_accept(night_event_t *ev);

int
night_enable_accept_events();

int
night_disable_accept_events();

void
night_close_accepted_connection(night_connection_t *c);

#endif /* _NIGHT_EVENT_ACCEPT_H_ */
