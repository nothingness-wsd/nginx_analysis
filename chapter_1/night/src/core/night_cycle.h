#ifndef _NIGHT_CYCLE_H_
#define _NIGHT_CYCLE_H_

struct night_cycle_s 
{
	night_pool_t				*pool;
	void						****conf_ctx;
	
	night_cycle_t				*old_cycle;
	
	night_array_t				paths;
	night_array_t				listening;
	
	night_str_t					conf_prefix;
	
    night_str_t					prefix;
    
    night_str_t					conf_file;
    
	night_array_t				config_dump;
    night_rbtree_t				config_dump_rbtree;
    night_rbtree_node_t			config_dump_sentinel;
    
	night_list_t				open_files;
	night_list_t				shared_memory;
	
	night_queue_t				reusable_connections_queue;
	
	night_str_t					hostname;
	
	night_module_t				**modules;
    size_t						modules_n;
    
    size_t						connection_n;

};

night_cycle_t *
night_init_cycle(night_cycle_t *old_cycle);

#endif /* _NIGHT_CYCLE_H_ */
