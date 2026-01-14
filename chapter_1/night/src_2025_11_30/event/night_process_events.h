#ifndef _NIGHT_PROCESS_EVENTS_H_
#define _NIGHT_PROCESS_EVENTS_H_

void
night_process_events_and_timers();

int 
night_process_events(int64_t timer, uint32_t flags);

#endif /* _NIGHT_PROCESS_EVENTS_H_ */
