#ifndef _NIGHT_CORE_CONF_H_
#define _NIGHT_CORE_CONF_H_

typedef struct night_core_conf_s night_core_conf_t;

struct night_core_conf_s
{
	int						worker_processes;
	char					*username;
    uid_t                 	user;
    gid_t					group;
    
	night_str_t				pid;
    night_str_t				oldpid;
};


#endif /* _NIGHT_CORE_CONF_H_ */
