#include "night_core.h"
#include "night_modules.h"
#include "night_module.h"
#include "night_cycle.h"

int night_modules_n;

night_module_t* night_modules[] = 
{
	&night_core_module,
	&night_events_module,
	&night_event_core_module,
	&night_http_module,
	&night_http_core_module,
	&night_epoll_module,
	&night_http_mirror_module,
	&night_http_log_module,
	&night_http_static_module,
	&night_http_autoindex_module,
	&night_http_index_module,
	&night_http_try_files_module,
	&night_http_auth_basic_module,
	&night_http_access_module,
	&night_http_limit_conn_module,
	&night_http_limit_req_module,
	&night_http_rewrite_module,
	&night_http_write_filter_module,
	&night_http_header_filter_module,
	&night_http_range_header_filter_module,
	&night_http_postpone_filter_module,
	&night_http_copy_filter_module,
	&night_http_range_body_filter_module,
	NULL
};

int
night_preinit_modules()
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_preinit_modules\n\n");
    
    int i;

    for (i = 0; night_modules[i]; i++) 
    {
        night_modules[i]->index = i;
        
        dprintf(trace_file_fd,"night_modules[%d]->name=%s\n",i,night_modules[i]->name);
        dprintf(trace_file_fd,"night_modules[%d]->index=%d\n\n",i,night_modules[i]->index);
    }

    night_modules_n = i;
    
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"night_modules_n=%d\n\n",night_modules_n);

    return NIGHT_OK;
}

int
night_count_modules(uint32_t type)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_count_modules\n\n");
	
	int 			i;
	int 			max = 0;
	int 			next = 0;
	night_module_t 	*module;
	
	for (i = 0; night_cycle->modules[i]; i++)
	{
		module = night_cycle->modules[i];
		
		if (module->type != type )
		{
			continue;
		}
		
		if (module->ctx_index != NIGHT_MODULE_UNSET_INDEX)
		{
			if (module->ctx_index > max )
			{
                max = module->ctx_index;
            }
            
            if (module->ctx_index == next) 
            {
                next++;
            }
            
            continue;
		}
		
		module->ctx_index = night_module_ctx_index(type, next);
        
        if( module->ctx_index > max )
        {
            max = module->ctx_index;
        }
        
        next = module->ctx_index + 1;
    }
    
    return max + 1;
}

int
night_module_ctx_index(uint32_t type, int index)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_module_ctx_index\n\n");
	
	int 			i;
	night_module_t	*module;

again:	
	for(i = 0; night_cycle->modules[i]; i++)
	{
		module = night_cycle->modules[i];
		
		if(module->type != type)
		{
			continue;
		}
		
		if(module->ctx_index == index)
		{
			index++;
			goto again;
		}
	}
	return index;
}
