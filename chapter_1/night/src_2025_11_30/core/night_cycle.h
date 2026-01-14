#ifndef _NIGHT_CYCLE_H_
#define _NIGHT_CYCLE_H_

#include "night_array.h"
#include "night_queue.h"

struct night_cycle_s
{
	night_pool_t		*pool;
	night_str_t			hostname;
	night_str_t			conf_file;
	night_array_t 		listening;
	
	int					night_modules_n;
	night_module_t		**modules;
	void				**conf_ctx;
	int					connection_n;
	int 				free_connection_n;
	int   				reusable_connections_n;
	night_queue_t 		reusable_connections_queue;
	
	uint64_t 			files_n;
    night_connection_t 	**files;
    
	night_event_t 		*read_events;
    night_event_t 		*write_events;
    night_connection_t 	*connections;
    night_connection_t 	*free_connections;
    

};

int
night_init_cycle();

int
night_cycle_modules(night_cycle_t *night_cycle);

#endif /* _NIGHT_CYCLE_H_ */
