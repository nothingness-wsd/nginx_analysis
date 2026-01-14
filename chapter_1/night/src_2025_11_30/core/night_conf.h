#ifndef _NIGHT_CONF_H_
#define _NIGHT_CONF_H_

#define NIGHT_CONF_OK 			(0)
#define NIGHT_CONF_BLOCK_START 	(1)
#define NIGHT_CONF_BLOCK_DONE 	(2)
#define NIGHT_CONF_FILE_DONE 	(3)

#include "night_array.h"

struct night_conf_s
{
	night_pool_t 		*pool;
	night_cycle_t 		*cycle;
	night_array_t 		args;
	night_conf_file_t 	*conf_file;
	void				**ctx; 
	
};

int
night_conf_parse(night_conf_t *cf, night_str_t *filename);

int 
night_conf_read_token(night_conf_t *cf);

int
night_conf_handler(night_conf_t *cf, int last);

int
night_conf_set_num(night_conf_t *cf, night_command_t *cmd, void *conf);

#endif /* _NIGHT_CONF_H_ */
