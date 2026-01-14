#include "night_core.h"
#include "night_cycle.h"
#include "night_pool.h"
#include "night_string.h"
#include "night_listening.h"
#include "night_module.h"
#include "night_core_module_ctx.h"
#include "night_conf.h"

int
night_init_cycle()
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_init_cycle\n\n");
    
    night_pool_t 			*pool;
    char 					hostname[NIGHT_HOST_NAME_LEN];
    int 					rc;
    char 					*rv;
    night_core_module_ctx_t	*module_ctx;
    int						i;
    night_conf_t			conf;
    night_listening_t		*ls;
    
    // create pool
    pool = night_create_pool(NIGHT_DEFAULT_POOL_SIZE);
    if(pool == NULL)
    {
    	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(error_log_fd,"night_create_pool(%d) failed\n\n", NIGHT_DEFAULT_POOL_SIZE);
        
        return NIGHT_ERROR;
    }
    
    night_cycle->pool = pool;
    
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"night_init_cycle create pool completed\n\n");
    
	// get hostname
    night_memzero(hostname,NIGHT_HOST_NAME_LEN);
    
    rc = gethostname(hostname, NIGHT_HOST_NAME_LEN - 1);
    if(rc == -1)
    {
    	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd,"gethostname failed\n\n");
    	
    	night_destroy_pool(pool);
    	return NIGHT_ERROR;
    }
    
    night_cycle->hostname.data = night_pmalloc(pool, strlen(hostname) + 1);
    if (night_cycle->hostname.data == NULL)
    {
    	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd,"night_pmalloc failed\n\n");
    	
    	night_destroy_pool(pool);
    	return NIGHT_ERROR;
    }
    
    night_strlow(night_cycle->hostname.data, hostname, strlen(hostname) + 1);
    night_cycle->hostname.len = strlen(hostname);
    
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "night_cycle->hostname.len=%ld\n", night_cycle->hostname.len);
    dprintf(trace_file_fd, "night_cycle->hostname.data=%s\n\n", night_cycle->hostname.data);
    
	// set configuration file
    night_cycle->conf_file.data = NIGHT_CONF_FILE;
    night_cycle->conf_file.len = strlen(NIGHT_CONF_FILE);
    
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "night_cycle->conf_file.len=%ld\n", night_cycle->conf_file.len);
    dprintf(trace_file_fd, "night_cycle->conf_file.data=%s\n\n", night_cycle->conf_file.data);
    
    // init listening
    if (night_array_init(&night_cycle->listening, pool, 4, sizeof(night_listening_t))
         != NIGHT_OK)
    {
		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd,"night_array_init failed\n\n");
    	
        night_destroy_pool(pool);
        return NIGHT_ERROR;
    }
    
    night_memzero(night_cycle->listening.elts, night_cycle->listening.nalloc * sizeof(night_listening_t));
    
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"night_init_cycle init listening completed\n\n");
    
    // init reusable_connections_queue
    night_queue_init(&night_cycle->reusable_connections_queue);
    
    // module's configuration context
    night_cycle->conf_ctx = night_pmalloc(pool, night_modules_n * sizeof(void*));
	
	// copy modules to this cycle
    night_cycle_modules(night_cycle);
    
	// create configuration struct for CORE_MODULE
    for (i = 0; night_cycle->modules[i]; i++)
    {
        if (night_cycle->modules[i]->type != NIGHT_CORE_MODULE)
        {
            continue;
        }
        module_ctx = night_cycle->modules[i]->ctx;
        
        if (module_ctx->create_conf)
        {
            rv = module_ctx->create_conf();
            if (rv == NULL)
            {
                night_destroy_pool(pool);
                return NIGHT_ERROR;
            }
            
            night_cycle->conf_ctx[night_cycle->modules[i]->index] = rv; 
        }
    }
    
    // configuration Parser init
    conf.pool = night_create_pool(NIGHT_DEFAULT_POOL_SIZE);
    conf.cycle = night_cycle;
    conf.ctx = night_cycle->conf_ctx;
    night_array_init(&conf.args, conf.pool, 10, sizeof(night_str_t));
    
	// configuration file parse
    if (night_conf_parse(&conf, &night_cycle->conf_file) != NIGHT_OK)
    {
        night_destroy_pool(conf.pool);
        night_destroy_pool(pool);
        
		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(error_log_fd,"night_conf_parse failed\n\n");
		
        return NIGHT_ERROR;
    } 
    
    night_destroy_pool(conf.pool);
    
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"configuration parse is completed\n\n");
    
    // NIGHT_CORE_MODULE init_conf
	for (i = 0; night_cycle->modules[i]; i++)
    {
        if (night_cycle->modules[i]->type != NIGHT_CORE_MODULE)
        {
            continue;
        }
        
        module_ctx = night_cycle->modules[i]->ctx;
        
        if (module_ctx->init_conf)
        {
            dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            dprintf(trace_file_fd,"module->init_conf\n");
            dprintf(trace_file_fd,"modules[%d]->name=%s\n\n", i, night_cycle->modules[i]->name);
            
            rc = module_ctx->init_conf(night_cycle->conf_ctx[night_cycle->modules[i]->index]);
            if (rc != NIGHT_OK)
            {
            	night_destroy_pool(pool);
            	
				dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
				dprintf(error_log_fd,"\"%s\" init_conf failed\n\n", night_cycle->modules[i]->name);
                
                return NIGHT_ERROR;
            }
        }
    }
    
    // create sockets, bind and listing ports
    rc = night_open_listening_sockets();
	if (rc != NIGHT_OK) 
    {
        goto failed;
    }
    
    // set socket 
	rc = night_configure_listening_sockets();
	if (rc != NIGHT_OK) 
    {
        goto failed;
    }

    return NIGHT_OK;
    
failed:    

	// close socket options
    ls = night_cycle->listening.elts;
    for (i = 0; i < night_cycle->listening.nelts; i++) 
    {
        if (ls[i].fd ==  -1 ) 
        {
            continue;
        }

        close(ls[i].fd);
    }

    night_destroy_pool(pool);

    return NIGHT_ERROR;
}

int
night_cycle_modules(night_cycle_t *night_cycle)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_cycle_modules\n\n");
    
    night_cycle->night_modules_n = night_modules_n;
    night_cycle->modules = night_pmalloc(night_cycle->pool, (night_modules_n + 1) * sizeof(night_module_t*));
    
    memcpy(night_cycle->modules, night_modules, night_modules_n * sizeof(night_module_t*));
               
    return NIGHT_OK;
}
