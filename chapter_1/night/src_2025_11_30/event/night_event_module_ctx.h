#ifndef _NIGHT_EVENT_MODULE_CTX_H_
#define _NIGHT_EVENT_MODULE_CTX_H_

#include "night_event_actions.h"

typedef struct night_event_module_ctx_s night_event_module_ctx_t;

struct night_event_module_ctx_s
{
	void* (*create_conf)();
	int   (*init_conf)(void *conf);
	night_event_actions_t actions;
};


#endif /* _NIGHT_EVENT_MODULE_CTX_H_ */
