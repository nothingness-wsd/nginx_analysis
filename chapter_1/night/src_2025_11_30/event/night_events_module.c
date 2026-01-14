#include "night_core.h"
#include "night_events_module.h"
#include "night_module.h"
#include "night_core_module_ctx.h"
#include "night_event_module_ctx.h"
#include "night_conf.h"
#include "night_pool.h"
#include "night_cycle.h"
#include "night_modules.h"
#include "night_listening.h"
#include "night_core_module.h"
#include "night_event.h"

night_command_t night_events_commands[] = 
{
	{
		night_string("events"),
		NIGHT_MAIN_CONF,
		0,
		night_events_block
	},
	night_null_command
}; 

night_core_module_ctx_t night_events_module_ctx = 
{
	NULL,
	night_event_init_conf
};

night_module_t night_events_module = 
{
    "night_events_module",
    NIGHT_MODULE_UNSET_INDEX,
    NIGHT_MODULE_UNSET_INDEX,
    NIGHT_CORE_MODULE,
    night_events_commands,
    &night_events_module_ctx,
  	NULL,
  	NULL,
  	NULL
};

int 					night_event_modules_n;

int
night_events_block(night_conf_t *cf, night_command_t *cmd, void *conf)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_events_block\n\n");
	
	void						***ctx;
	int 						i;
	night_module_t 				*module;
	night_event_module_ctx_t	*module_ctx;
	night_conf_t 				pcf;
	int 						rc;
	
	if ( *(void**) conf)
	{
		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(error_log_fd, "events block configuration is duplicate\n\n");
		
		return NIGHT_ERROR;
	}
	
	ctx = night_pmalloc(night_cycle->pool, sizeof(void*));
	if (ctx == NULL)
	{
		dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(error_log_fd, "night_pmalloc failed to allocate memory for events block configuration\n\n");
		
		return NIGHT_ERROR;
	}
	
	night_event_modules_n = night_count_modules(NIGHT_EVENT_MODULE);
	
	*ctx = night_pmalloc(night_cycle->pool, night_event_modules_n * sizeof(void*));
	if ((*ctx) == NULL)
	{
		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(error_log_fd, "night_pmalloc failed to allocate memory for events block configuration\n\n");
		
		return NIGHT_ERROR;
	}
	
	*(void**) conf = ctx;
	
	for (i = 0; cf->cycle->modules[i]; i++)
	{
        if (cf->cycle->modules[i]->type != NIGHT_EVENT_MODULE ) 
        {
            continue;
        }
		
		module = cf->cycle->modules[i];
        module_ctx = cf->cycle->modules[i]->ctx;

        if (module_ctx->create_conf) 
        {
            (*ctx)[cf->cycle->modules[i]->ctx_index] = module_ctx->create_conf();      
            if ((*ctx)[cf->cycle->modules[i]->ctx_index] == NULL) 
            {
            	dprintf(error_log_fd,"FILE=%s:LINE=%d\n",__FILE__, __LINE__);
            	dprintf(error_log_fd, "%s create_conf failed while night_events_block\n\n", module->name);
            	
                return NIGHT_ERROR;
            }
        }
    }
    
    pcf = *cf;
    
    cf->ctx = (void**) ctx;
    
    rc = night_conf_parse(cf, NULL);
    
    *cf = pcf;
    
    
	for (i = 0; cf->cycle->modules[i]; i++) 
	{
        if(night_cycle->modules[i]->type != NIGHT_EVENT_MODULE) 
        {
            continue;
        }

		module = night_cycle->modules[i];
        module_ctx = night_cycle->modules[i]->ctx;
       
        if (module_ctx->init_conf) 
        {   
            rc = module_ctx->init_conf((*ctx)[night_cycle->modules[i]->ctx_index]);
            if (rc != NIGHT_OK) 
            {
				dprintf(error_log_fd,"FILE=%s:LINE=%d\n",__FILE__, __LINE__);
				dprintf(error_log_fd, "%s init_conf failed while night_events_block\n\n", module->name);
                
                return rc;
            }
        }
    }
    
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"night_events_block parse completed\n" "rc=%d\n\n", rc);
    
    return rc;
}


int
night_event_init_conf(void *conf)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function:\t" "night_event_init_conf\n\n");
	
//	night_core_conf_t 	*ccf;
	night_listening_t 	*ls;
	int 				i;
	int 				rc;
	
	if (night_cycle->connection_n < night_cycle->listening.nelts + 1) 
	{
		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(error_log_fd,"%d worker_connections are not enough for %ld listening sockets\n\n",
				night_cycle->connection_n, night_cycle->listening.nelts);
		
		return NIGHT_ERROR;		
	}
	
	//ccf = (night_core_conf_t*) night_get_conf(night_cycle->conf_ctx, night_core_module);
	
	ls = night_cycle->listening.elts;
	for (i = 0; i < night_cycle->listening.nelts; i++) 
	{
		if (ls[i].worker != 0)
		{
			continue;
		}
		
		rc = night_clone_listening(&ls[i]);
		if(rc != NIGHT_OK) 
		{
			return NIGHT_ERROR;
		}

		// cloning may change listening.elts 
		ls = night_cycle->listening.elts;
	}
	
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"night_cycle->listening.nelts = %ld\n\n", night_cycle->listening.nelts);
	
	return NIGHT_OK;
}
