#ifndef _NIGHT_MODULE_H_
#define _NIGHT_MODULE_H_

#define NIGHT_CORE_MODULE		0x45524F43  /* "CORE" */
#define NIGHT_CONF_MODULE		0x464E4F43  /* "CONF" */
#define NIGHT_EVENT_MODULE		0x544E5645  /* "EVNT" */
#define NIGHT_HTTP_MODULE		0x50545448	/* "HTTP" */

typedef struct night_module_s 		night_module_t;

struct night_module_s
{
	char* 			name;
	int				index;
	int				ctx_index;
	uint64_t		type;
	void			*ctx;
	night_command_t	*commands;
};


int
night_preinit_modules(void);

#endif /* _NIGHT_MODULE_H_ */
