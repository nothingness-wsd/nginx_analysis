#ifndef _NIGHT_EVENT_CORE_MODULE_H_
#define _NIGHT_EVENT_CORE_MODULE_H_

typedef struct night_event_core_conf_s night_event_core_conf_t;

struct night_event_core_conf_s
{
	int connections;
	int use;
};

void*
night_event_core_create_conf();

int		
night_event_connections(night_conf_t *cf, night_command_t *cmd, void *conf);

int
night_event_core_init_conf(void *conf);

int
night_event_core_process_init();

#endif /* _NIGHT_EVENT_CORE_MODULE_H_ */
