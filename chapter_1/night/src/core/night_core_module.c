#include "night_core.h"


static void *
night_core_module_create_conf(night_cycle_t *cycle)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "%s\n\n", __func__);
	
	night_core_conf_t  *ccf;
	
	ccf = night_pcalloc(cycle->pool, sizeof(night_core_conf_t));
    if (ccf == NULL) 
    {
        return NULL;
    }
    
    ccf->worker_processes = NIGHT_CONF_UNSET;
	ccf->user = (uid_t) NIGHT_CONF_UNSET_UINT;
    ccf->group = (gid_t) NIGHT_CONF_UNSET_UINT;
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function %s:\t" "return\n\n", __func__);
	
    return ccf;
}

static char *
night_core_module_init_conf(night_cycle_t *cycle, void *conf)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "%s\n\n", __func__);
	
	night_core_conf_t *ccf = conf;
	
	night_conf_init_value(ccf->worker_processes, 1);
	
	if (ccf->pid.len == 0) 
	{
        night_str_set(&ccf->pid, NIGHT_PID_PATH);
        
        dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(trace_file_fd, "pid file=%s\n\n", ccf->pid.data);
    }
	
	if (night_conf_full_name(cycle, &ccf->pid, 0) != NIGHT_OK) 
	{
        return NIGHT_CONF_ERROR;
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "pid file full name=%s\n\n", ccf->pid.data);
	
	ccf->oldpid.len = ccf->pid.len + sizeof(NIGHT_OLDPID_EXT);
	
	ccf->oldpid.data = night_pnalloc(cycle->pool, ccf->oldpid.len);
    if (ccf->oldpid.data == NULL) 
    {
        return NIGHT_CONF_ERROR;
    }
    
	memcpy(night_cpymem(ccf->oldpid.data, ccf->pid.data, ccf->pid.len),
			NIGHT_OLDPID_EXT, sizeof(NIGHT_OLDPID_EXT));
			
	               
   	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
   	dprintf(trace_file_fd, "oldpid file=%s\n\n", ccf->oldpid.data);  		
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function %s:\t" "return\n\n", __func__);
}

static char *
night_set_worker_processes(night_conf_t *cf, night_command_t *cmd, void *conf)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "%s\n\n", __func__);
	
	night_str_t				*value;
    night_core_conf_t		*ccf;
    
	ccf = (night_core_conf_t *) conf;

    if (ccf->worker_processes != NIGHT_CONF_UNSET) 
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "function %s:\t" "return \"is duplicate\"\n\n", __func__);
    	
        return "is duplicate";
    }
    
	value = cf->args->elts;

    if (strcmp(value[1].data, "auto") == 0) 
    {
        ccf->worker_processes = night_ncpu;
        
        dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(trace_file_fd, "worker_processes=%d\n", ccf->worker_processes);
        dprintf(trace_file_fd, "function %s:\t" "return NIGHT_CONF_OK\n\n", __func__);
        
        return NIGHT_CONF_OK;
    }
    
    ccf->worker_processes = night_atoi(value[1].data, value[1].len);

    if (ccf->worker_processes == NIGHT_ERROR) 
    {
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "function %s:\t" "return \"invalid value\"\n\n", __func__);
    	
        return "invalid value";
    }

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function %s:\t" "return NIGHT_CONF_OK\n\n", __func__);
	
    return NIGHT_CONF_OK;
}

static 
night_core_module_ctx_t  night_core_module_ctx = 
{
    night_string("core"),
    night_core_module_create_conf,
    night_core_module_init_conf
};


static 
night_command_t  night_core_commands[] = 
{
    {
     	night_string("worker_processes"),               			// name
      	NIGHT_MAIN_CONF| NIGHT_DIRECT_CONF | NIGHT_CONF_TAKE1,		// type
      	night_set_worker_processes,									// set
      	0,															// conf
      	0,															// offset
      	NULL 														// post
    },

	night_null_command
};

night_module_t  night_core_module = 
{
    "night_core_module",		// name
    NIGHT_MODULE_UNSET_INDEX,	// index
    NIGHT_MODULE_UNSET_INDEX,	// ctx_index
    NIGHT_CORE_MODULE,			// type
    &night_core_module_ctx,		// module context
    night_core_commands			// module directives     
};


