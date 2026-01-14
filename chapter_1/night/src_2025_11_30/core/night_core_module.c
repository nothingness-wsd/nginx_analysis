#include "night_core.h"
#include "night_core_module.h"
#include "night_module.h"
#include "night_core_module_ctx.h"
#include "night_pool.h"
#include "night_cycle.h"
#include "night_conf.h"
#include "night_string.h"

night_command_t night_core_commands[] = 
{
    {
        night_string("worker_processes"),
        NIGHT_DIRECT_CONF,
        0,
        night_set_worker_processes
    },
    night_null_command
};

night_core_module_ctx_t night_core_module_ctx = 
{
    night_core_module_create_conf,
    night_core_module_init_conf
};

night_module_t night_core_module = 
{
	"night_core_module",
	NIGHT_MODULE_UNSET_INDEX,
	NIGHT_MODULE_UNSET_INDEX,
	NIGHT_CORE_MODULE,
	night_core_commands,
	&night_core_module_ctx,
	NULL,
	NULL,
	NULL
};

void*
night_core_module_create_conf()
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_core_module_create_conf\n\n");
    
	night_core_conf_t *core_conf; 
	
	core_conf = night_pmalloc(night_cycle->pool, sizeof(night_core_conf_t));
	
	core_conf->worker_processes = NIGHT_CONF_INT_UNSET;
	
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"worker_processes=%d\n\n", core_conf->worker_processes);
	
	core_conf->uid = NIGHT_CONF_INT_UNSET;
	core_conf->euid = NIGHT_CONF_INT_UNSET;
	core_conf->gid = NIGHT_CONF_INT_UNSET;
    
    return core_conf;
}

int
night_set_worker_processes(night_conf_t *cf, night_command_t *cmd, void *conf)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_set_worker_processes\n\n");
    
    night_core_conf_t 	*core_conf;
    night_str_t			*value; 
    
    core_conf = (night_core_conf_t*) conf;
    
    if (core_conf->worker_processes != NIGHT_CONF_INT_UNSET)
    {
    	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd,"worker_processes configuraton is duplicate\n\n");
    	
    	return NIGHT_ERROR;
    }
    
    value = cf->args.elts;
    
    core_conf->worker_processes = night_atoi(value[1].data, value[1].len);
    
    if (core_conf->worker_processes == NIGHT_ERROR)
    {
    	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd,"invalid worker_processes configuration value\n\n");
    	
    	return NIGHT_ERROR;
    }
    
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"core_conf->worker_processes=%d\n\n", core_conf->worker_processes);
  
	return NIGHT_OK;
}

int
night_core_module_init_conf(void *conf)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_core_module_init_conf\n\n");
    
    night_core_conf_t	*cc;
	struct group  		*grp;
    struct passwd 		*pwd;
    uid_t				uid;
    uid_t				euid;
    
    
    cc = (night_core_conf_t*) conf;
    if (cc->pidfile.len == 0)
    {   
    	cc->pidfile.len = strlen(NIGHT_PIDFILE) + night_work_directory.len;
    	cc->pidfile.data = night_pmalloc(night_cycle->pool, cc->pidfile.len + 1);
    	if (cc->pidfile.data == NULL)
    	{
			dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(error_log_fd,"night_pmalloc failed\n\n");
    		
    		return NIGHT_ERROR;
    	}
    	
    	strcat(cc->pidfile.data, night_work_directory.data);
    	strcat(cc->pidfile.data, NIGHT_PIDFILE);
    }
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "cc->pidfile.len=%ld\n", cc->pidfile.len);
    dprintf(trace_file_fd, "cc->pidfile.data=%s\n\n", cc->pidfile.data);
    
    
	if (cc->uid == (uid_t) NIGHT_CONF_INT_UNSET && geteuid() == 0) 
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "euid == 0\n\n");
    	
        pwd = getpwnam(NIGHT_USER);
        if (pwd == NULL) 
        {   
        	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            dprintf(error_log_fd,"getpwnam(\"" NIGHT_USER "\") failed\n\n");
            
            return NIGHT_ERROR;
        }
        
        cc->username = NIGHT_USER;
        cc->uid = pwd->pw_uid;
        
        grp = getgrnam(NIGHT_GROUP);
        if (grp == NULL) 
        {
        	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            dprintf(error_log_fd,"getgrnam(\"" NIGHT_USER "\") failed\n\n");
            
            return NIGHT_ERROR;
        }

        cc->gid = grp->gr_gid;
    }

	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"euid=%u\n", cc->euid);
    dprintf(trace_file_fd,"uid=%u\n", cc->uid);
    dprintf(trace_file_fd,"username=%s\n", cc->username);
	dprintf(trace_file_fd,"gid=%u\n\n", cc->gid);
    
	return NIGHT_OK;
}
