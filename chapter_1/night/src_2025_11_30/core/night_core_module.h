#ifndef _NIGHT_CORE_MODULE_H_
#define _NIGHT_CORE_MODULE_H_

#define NIGHT_USER 		"nobody"
#define NIGHT_GROUP 	"nogroup"


typedef struct night_core_conf_s night_core_conf_t;

struct night_core_conf_s
{
    int 			worker_processes;
    
	night_str_t 	pidfile; 
//    char			**environment;
//    night_array_t 	env;
    char 			*username;
    uid_t 			uid; 
    uid_t 			euid; 
    gid_t 			gid;

};

void*
night_core_module_create_conf();

int
night_set_worker_processes(night_conf_t *cf, night_command_t *cmd, void *conf);

int
night_core_module_init_conf(void *conf);

#endif /* _NIGHT_CORE_MODULE_H_ */
