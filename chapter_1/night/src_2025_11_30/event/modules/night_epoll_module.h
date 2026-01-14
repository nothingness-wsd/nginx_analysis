#ifndef _NIGHT_EPOLL_MODULE_H_
#define _NIGHT_EPOLL_MODULE_H_

typedef struct night_epoll_conf_s night_epoll_conf_t;

struct night_epoll_conf_s 
{
	int events;
};

void*
night_epoll_create_conf();

int
night_epoll_init_conf(void *conf);

int
night_epoll_init();

int
night_epoll_add_event(night_event_t *ev, int event, uint32_t flags);

int
night_epoll_del_event(night_event_t *ev, int event, uint32_t flags);

int 
night_epoll_process_events(int64_t timer, uint32_t flags);

int
night_epoll_del_conn(night_connection_t *c, uint32_t flags);

#endif /* _NIGHT_EPOLL_MODULE_H_ */
