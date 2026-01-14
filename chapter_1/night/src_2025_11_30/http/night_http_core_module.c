#include "night_core.h"
#include "night_http_core_module.h"
#include "night_module.h"
#include "night_http_module_ctx.h"
#include "night_http_module.h"
#include "night_pool.h"
#include "night_cycle.h"
#include "night_conf.h"
#include "night_string.h"
#include "night_conf_file.h"
#include "night_file_name.h"
#include "night_http_port.h"
#include "night_http_listen_opt.h"
#include "night_http_request.h"
#include "night_connection.h"

night_http_module_ctx_t night_http_core_module_ctx = 
{
    night_http_create_main_conf,
    night_http_create_srv_conf,
    night_http_create_loc_conf
};

night_command_t night_http_core_commands[] = 
{
	{
		night_string("server"),
		NIGHT_HTTP_MAIN_CONF,
		0,
		night_http_server
	},
	{
        night_string("listen"),
        NIGHT_HTTP_SRV_CONF,
        NIGHT_HTTP_SRV_CONF_OFFSET,
        night_http_listen
    },
    {
        night_string("server_name"),
        NIGHT_HTTP_SRV_CONF,
        NIGHT_HTTP_SRV_CONF_OFFSET,
        night_http_server_name
    },
    {
        night_string("location"),
        NIGHT_HTTP_SRV_CONF,
        NIGHT_HTTP_SRV_CONF_OFFSET,
        night_http_location 
    },
    {
        night_string("root"),
        NIGHT_HTTP_LOC_CONF,
        NIGHT_HTTP_LOC_CONF_OFFSET,
        night_http_root
    },
    {
        night_string("error_page"),
        NIGHT_HTTP_SRV_CONF | NIGHT_HTTP_LOC_CONF,
        NIGHT_HTTP_LOC_CONF_OFFSET,
        night_http_error_page
    },
	night_null_command
};

night_module_t night_http_core_module = 
{
    "night_http_core_module",
    NIGHT_MODULE_UNSET_INDEX,
    NIGHT_MODULE_UNSET_INDEX,
    NIGHT_HTTP_MODULE,       
    night_http_core_commands,  
	&night_http_core_module_ctx,
	NULL,
	NULL,
	NULL
};

static night_str_t  night_http_core_text_html_type = night_string("text/html");
static night_str_t  night_http_core_image_gif_type = night_string("image/gif");
static night_str_t  night_http_core_image_jpeg_type = night_string("image/jpeg");

static night_hash_key_t  night_http_core_default_types[] = 
{
    { night_string("html"), 0, &night_http_core_text_html_type },
    { night_string("gif"), 	0, &night_http_core_image_gif_type },
    { night_string("jpg"), 	0, &night_http_core_image_jpeg_type },
    { night_null_string, 	0, NULL }
};


void*
night_http_create_main_conf(night_conf_t *cf)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_http_create_main_conf\n\n");

    night_http_core_main_conf_t *mc;
    
    mc = night_pmalloc(night_cycle->pool, sizeof(night_http_core_main_conf_t));
    if (mc == NULL )
    {
    	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(error_log_fd, "night_pmalloc failed\n\n");
        
        return NULL;
    }
    
    if (night_array_init(&mc->servers, night_cycle->pool, 4, sizeof(night_http_core_srv_conf_t*))
        != NIGHT_OK)
    {
    	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd, "night_array_init failed to init night_http_main_conf_t's servers while night_http_create_main_conf\n\n");
        
        return NULL;
    }
    
    if (night_array_init(&mc->ports, night_cycle->pool, 4, sizeof(night_http_port_t))
        != NIGHT_OK)
    {
    	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd, "night_array_init failed to init night_http_main_conf_t's ports while night_http_create_main_conf\n\n");
        
        return NULL;
    }
    
    
/*	if (night_array_init(&mc->variables, night_cycle->pool, 4, sizeof(night_variable_t))
            != NIGHT_OK)
	{
		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd, "night_array_init failed to init night_http_main_conf_t's variables while night_http_create_main_conf\n\n");
        
        return NULL;
	}            
*/    
    return mc;    
}

void*
night_http_create_srv_conf(night_conf_t *cf)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_http_create_srv_conf\n\n");
	
	night_http_core_srv_conf_t *sc;
    
    sc = night_pmalloc(night_cycle->pool, sizeof(night_http_core_srv_conf_t));
    if (sc == NULL)
    {
    	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd, "night_pmalloc failed to allocate memory for night_http_srv_conf_t while night_http_create_srv_conf\n\n");
        
        return NULL;
    }
    
    sc->client_header_timeout = 60000;
    
    sc->client_header_buffer_size = 1024;
    
    return sc;
}

void*
night_http_create_loc_conf(night_conf_t *cf)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_http_create_loc_conf\n\n");
	
	night_http_core_loc_conf_t		*lc;
	int 							rc;
	night_hash_init_t   			types_hash;
	uint64_t						i;
	night_hash_key_t   				*type;
    
    lc = night_pmalloc(night_cycle->pool, sizeof(night_http_core_loc_conf_t));
    if (lc == NULL)
    {
    	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd, "night_pmalloc failed to allocate memory for night_http_loc_conf_t while night_http_create_loc_conf\n\n");
        
        return NULL;
    }
    
    lc->keepalive_timeout = 75000;
    
    rc = night_array_init(&lc->error_pages, night_cycle->pool, 10, sizeof(night_http_error_page_t));
    if (rc != NIGHT_OK)
    {
    	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd,"night_array_init failed while night_http_create_loc_conf\n\n");
    	
    	return NULL;
    }
    
    //  MIME 类型哈希表最大桶数，默认 1024
    lc->types_hash_max_size = 1024;
    	
	// 哈希桶大小，默认 64 字节
	lc->types_hash_bucket_size = 64;

	if (lc->types == NULL) 
	{
		dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd,"if (lc->types == NULL)\n\n");
    	
		lc->types = night_array_create(night_cycle->pool, 3, sizeof(night_hash_key_t));
        if (lc->types == NULL) 
        {
            return NULL;
        }
        
		for (i = 0; night_http_core_default_types[i].key.len; i++) 
		{
            type = night_array_push(lc->types);
            if (type == NULL) 
            {
                return NULL;
            }

            type->key = night_http_core_default_types[i].key;
            
            type->key_hash = night_hash_key_lc(night_http_core_default_types[i].key.data, night_http_core_default_types[i].key.len);
            
            type->value = night_http_core_default_types[i].value;
        }
	}
	
	if (lc->types_hash.buckets == NULL) 
	{
		
		dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd,"if (lc->types_hash.buckets == NULL)\n\n");
    	
        types_hash.hash = &lc->types_hash;
        types_hash.key = night_hash_key_lc;
        types_hash.max_size = lc->types_hash_max_size;
        types_hash.bucket_size = lc->types_hash_bucket_size;
        types_hash.name = "types_hash";
        types_hash.pool = night_cycle->pool;
        types_hash.temp_pool = NULL;

        if (night_hash_init(&types_hash, lc->types->elts, lc->types->nelts) != NIGHT_OK)
        {
            return NULL;
        }
    }
    
	lc->default_type.data = "text/plain";
	lc->default_type.len = sizeof("text/plain");
    
    return lc;
}

int
night_http_server(night_conf_t	*cf, night_command_t *cmd, void *conf)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_http_server\n\n");
	
	night_http_conf_t				*hc;
	night_http_conf_t				*http_ctx;
	int 							i;
	night_module_t					*module;
	night_http_module_ctx_t			*module_ctx;
	int 							mi;
	night_http_core_srv_conf_t		*sc;
	int 							ctx_index;
	night_http_core_srv_conf_t		**psc;
	night_http_core_main_conf_t		*mc;
	night_conf_t 					pcf;
	int 							rc;
	
	hc = night_pmalloc(night_cycle->pool, sizeof(night_http_conf_t));
    if (hc == NULL)
    {
    	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd, "night_pmalloc failed to allocate memory for night_http_conf_t while night_http_server\n\n");
        
        return NIGHT_ERROR;
    }
    
    http_ctx = (night_http_conf_t*) cf->ctx;
    
    hc->main_conf = http_ctx->main_conf;
    
    hc->srv_conf = night_pmalloc(night_cycle->pool, sizeof(void*) * night_http_modules_n);
    if (hc->srv_conf == NULL)
    {
    	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd, "night_pmalloc failed to allocate memory for night_http_conf_t's srv_conf while night_http_server\n\n");
        
        return NIGHT_ERROR;
    }
	
	hc->loc_conf = night_pmalloc(night_cycle->pool, sizeof(void*) * night_http_modules_n);
    if (hc->srv_conf == NULL)
    {
    	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd, "night_pmalloc failed to allocate memory for night_http_conf_t's loc_conf while night_http_server\n\n");
        
        return NIGHT_ERROR;
    }
    
    for (i = 0; cf->cycle->modules[i]; i++)
    {
        if (cf->cycle->modules[i]->type != NIGHT_HTTP_MODULE )
        {
            continue;
        }
        
        module = cf->cycle->modules[i];
        module_ctx = module->ctx;
        mi = module->ctx_index;
        
        if (module_ctx->create_srv_conf)
        {
        	hc->srv_conf[mi] = module_ctx->create_srv_conf(cf);
        	if (hc->srv_conf[mi] == NULL)
        	{
        		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        		dprintf(error_log_fd, "\"%s\" create_srv_conf failed while night_http_server\n\n",module->name);
        	 	
        	 	return NIGHT_ERROR;
        	}
        }
        
        if (module_ctx->create_loc_conf)
        {
        	hc->loc_conf[mi] = module_ctx->create_loc_conf(cf);
        	if (hc->loc_conf[mi] == NULL)
        	{
        		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        		dprintf(error_log_fd, "\"%s\" create_loc_conf failed while night_http_server\n\n",module->name);
        	 	
        	 	return NIGHT_ERROR;
        	}
        }
    }
    
    ctx_index = night_http_core_module.ctx_index;
    
    sc = hc->srv_conf[ctx_index];
    sc->ctx = hc;
    
    mc = hc->main_conf[ctx_index];
    
    psc = night_array_push(&mc->servers);
    if (psc == NULL )
    {
    	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd,"night_array_push failed while night_http_server\n\n");
        
        return NIGHT_ERROR;
    }
    
    *psc = sc;
    
    pcf = *cf;
    
    cf->ctx = (void**) hc;
    
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"night_http_server:\nrc = night_conf_parse(cf, NULL);\n\n");
    
    rc = night_conf_parse(cf, NULL);

    *cf = pcf;
    
	return rc;
}

int
night_http_listen(night_conf_t *cf, night_command_t *cmd, void *conf)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function:\t" "night_http_listen\n\n");
	
	night_str_t											*value;
	socklen_t 											socklen;
	struct sockaddr_in 									*sin;
	int 												n;
	in_port_t 											port;
	night_http_listen_opt_t								*lsopt; 
	int 												rc;
	night_http_core_srv_conf_t							*sc;
	
	
	sc = (night_http_core_srv_conf_t*) conf;
	
	value = cf->args.elts;
	
	socklen = sizeof(struct sockaddr_in);
	
	sin = night_pmalloc(night_cycle->pool, socklen);
	if (sin == NULL)
	{
		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(error_log_fd,"night_pmalloc failed to allocate memory for sockaddr_in while night_http_listen\n\n");
		
		return NIGHT_ERROR;
	}
	
	sin->sin_family = AF_INET;
	
	n = night_atoi(value[1].data, value[1].len);
	
	if (n == NIGHT_ERROR)
	{
		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(error_log_fd,"listen configuration value is invalid\n\n");
        
        return NIGHT_ERROR;
    }
    
    if (n < 1 || n > 65535) 
    {
    	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(error_log_fd,"listen port must greater than 1, less than 65535\n\n");
        
        return NIGHT_ERROR;
    }
	
	port = (in_port_t) n;
	
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"port=%d\n\n",port);
	
	sin->sin_port = htons(port);
	
	sin->sin_addr.s_addr = INADDR_ANY; 
	
	lsopt = night_pmalloc(cf->pool, sizeof(night_http_listen_opt_t));
	if (lsopt == NULL)
	{
	    dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(error_log_fd,"night_pmalloc failed to allocate memory for night_http_listen_opt_t while night_http_listen\n\n");
        
        return NIGHT_ERROR;
	}
	
	lsopt->backlog = NIGHT_LISTEN_BACKLOG;
	lsopt->sockaddr = (struct sockaddr*) sin;
    lsopt->socklen = socklen;
    
    rc = night_http_add_listen(cf, sc, lsopt);
	
	return rc;
}

int
night_http_add_listen(night_conf_t *cf, night_http_core_srv_conf_t *sc, night_http_listen_opt_t *lsopt)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function:\t" "night_http_add_listen\n\n");
	
	night_http_core_main_conf_t	*mc;
	struct sockaddr				*sa;
	in_port_t 					p;
	night_http_port_t			*ports;
	night_http_port_t			*port;
	int 						i;
	
	sa = lsopt->sockaddr;
    p = night_get_port(sa);
    
    mc = night_get_main_conf(cf, night_http_core_module);
    ports = mc->ports.elts;
    
    for (i = 0; i < mc->ports.nelts; i++)
    {
        if(p != ports[i].port || sa->sa_family != ports[i].family )
        {
            continue;
        }
        
        dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(error_log_fd, "listen port configuration is duplicate\n\n" );
        
        return NIGHT_ERROR;
    }
    
    port = night_array_push(&mc->ports);
    if (port == NULL)
    {
    	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd,"night_array_push failed for port while night_http_add_listen\n\n");
        
        return NIGHT_ERROR;
    }
    
    port->family = sa->sa_family;
    port->port = p;
    port->listen_opt = lsopt;
    port->server = sc;
	
	return NIGHT_OK;
}

in_port_t
night_get_port(struct sockaddr *sa)
{
	struct sockaddr_in *sin;
	
    sin = (struct sockaddr_in*) sa;
    
    return ntohs(sin->sin_port);
}


int
night_http_server_name(night_conf_t *cf, night_command_t *cmd, void *conf)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function:\t" "night_http_server_name\n\n");
	
	night_str_t					*value; 
	night_http_core_srv_conf_t	*sc;
	night_http_server_name_t	*server_name;
	int 						n;
	
	sc = (night_http_core_srv_conf_t*) conf;
	
	value = cf->args.elts;
	
	server_name = &(sc->server_name);
	
    server_name->server = sc;
    server_name->name.len = value[1].len;
    
    server_name->name.data = night_pmalloc(night_cycle->pool, value[1].len + 1);
    if (server_name->name.data == NULL)
    {
    	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd,"night_pmalloc failed while night_http_server_name\n\n");
    	
    	return NIGHT_ERROR;
    }
    
    night_strlow(server_name->name.data, value[1].data, server_name->name.len);
    
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"server_name->name.len=%ld\n", server_name->name.len);
    dprintf(trace_file_fd,"server_name->name.data=%s\n\n", server_name->name.data);
	
	return NIGHT_OK;
}

int
night_http_location(night_conf_t *cf, night_command_t *cmd, void *conf)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function:\t" "night_http_location\n\n");
	
	night_http_conf_t							*hc;
	night_http_conf_t							*phc;
	int 										i;
	night_module_t								*module;
	night_http_module_ctx_t						*module_ctx;
	int 										mi;
	night_http_core_loc_conf_t					*lc;
	night_http_core_loc_conf_t					*plc;
	int 										ctx_index;
	night_str_t									*value;
	int 										n;
	night_str_t									*name;
	int 										rc;
	night_conf_t 								save;
	
	phc = (night_http_conf_t*) cf->ctx;
	
	hc = night_pmalloc(night_cycle->pool, sizeof(night_http_conf_t));
    if (hc == NULL) 
    {
    	dprintf(error_log_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd, "night_pmalloc failed while night_http_location\n\n");
        return NIGHT_ERROR;
        
    }
    
    hc->main_conf = phc->main_conf;
    hc->srv_conf  = phc->srv_conf;
    
    hc->loc_conf = night_pmalloc(night_cycle->pool, sizeof(void*) * night_http_modules_n);
    if (hc->loc_conf == NULL) 
    {
        dprintf(error_log_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd, "night_pmalloc failed while night_http_location\n\n");
        
        return NIGHT_ERROR;
    }
    
    for (i = 0; cf->cycle->modules[i]; i++) 
    {
        if (cf->cycle->modules[i]->type != NIGHT_HTTP_MODULE) 
        {
            continue;
        }

        module = cf->cycle->modules[i];
        module_ctx = module->ctx;
		mi = module->ctx_index;
		
        if(module_ctx->create_loc_conf) 
        {
            hc->loc_conf[mi] = module_ctx->create_loc_conf(cf);
            if (hc->loc_conf[mi] == NULL) 
            {
            	dprintf(error_log_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            	dprintf(error_log_fd, "%s create_loc_conf failed\n\n", module->name);
                
                return NIGHT_ERROR;
            }
        }
    }
    
    ctx_index = night_http_core_module.ctx_index;
    
    lc = hc->loc_conf[ctx_index];
    lc->loc_conf = hc->loc_conf;
    lc->exact_match = 0;
    
    value = cf->args.elts;
    n = cf->args.nelts;
    
    if(n == 3)
    {
    	if(value[1].len == 1 && value[1].data[0] == '=' )
    	{
    		name = &value[2];
    		lc->exact_match = 1;
    	}
    	else
    	{
    		dprintf(error_log_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    		dprintf(error_log_fd, "invalid location \"%s\" at line %ld\n\n", value[1].data, cf->conf_file->line);
    	}
    }
    else
    {
    	name = &value[1];
    	if(name->data[0] == '=') 
    	{
            name->len -= 1;
            name->data += 1;
            lc->exact_match = 1;
        }
    }
    
    lc->name.len = name->len;
    lc->name.data = night_pmalloc(night_cycle->pool, name->len + 1);
    strcat(lc->name.data, name->data);
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "location name.len=%ld\n",lc->name.len);
    dprintf(trace_file_fd, "location name=%s\n\n",lc->name.data);
    
    plc = phc->loc_conf[ctx_index];
    
    rc = night_http_add_location(cf, &plc->locations, lc);
    
    if(rc != NIGHT_OK)
    {
    	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd,"night_http_add_location failed while night_http_location\n\n");
    	
    	return NIGHT_ERROR;
    }
    
    save = *cf;
    
    cf->ctx = (void*) hc;
    rc = night_conf_parse(cf, NULL);

    *cf = save;
   
	return rc;
}

int
night_http_add_location(night_conf_t *cf, night_queue_t **locations, night_http_core_loc_conf_t *lc)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function:\t" "night_http_add_location\n\n");
	
	night_http_location_queue_t *lq;
	
	if (*locations == NULL) 
	{
		*locations = night_pmalloc(night_cycle->pool, sizeof(night_http_location_queue_t));
		if (*locations == NULL)
		{
		    dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    		dprintf(error_log_fd,"night_pmalloc failed while night_http_add_location\n\n");
			
			return NIGHT_ERROR;
		}
		
		night_queue_init(*locations);
	}
	
	lq = night_pmalloc(night_cycle->pool, sizeof(night_http_location_queue_t));
    if (lq == NULL) 
    {
    	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd,"night_pmalloc failed while night_http_add_location\n\n");
        
        return NIGHT_ERROR;
    }
    
    if (lc->exact_match)
    {
        lq->exact = lc;
        lq->inclusive = NULL;

    } 
    else 
    {
        lq->exact = NULL;
        lq->inclusive = lc;
    }
    
    lq->name = &lc->name;
    lq->file_name = cf->conf_file->file.filename.data;
    lq->line = cf->conf_file->line;
	
	night_queue_init(&lq->list);
	
	night_queue_insert_tail(*locations, &lq->queue);
	
	return NIGHT_OK;
}

int
night_http_root(night_conf_t *cf, night_command_t *cmd, void *conf)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function:\t" "night_http_root\n\n");
	
	night_http_core_loc_conf_t	*lc;
    night_str_t 				*value;
    int 						rc;
    
    lc = (night_http_core_loc_conf_t*) conf;
    if (lc->root.data) 
    {
    	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(error_log_fd,"root configuration is duplicate at line %ld\n\n", cf->conf_file->line);
        
        return NIGHT_ERROR;
    }
    
    value = cf->args.elts;
    lc->root = value[1];
    
    rc = night_get_full_name(&lc->root);
    if (rc != NIGHT_OK)
    {
    	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd,"night_get_full_name failed for root while night_http_root\n\n");
    	
    	return NIGHT_ERROR;
    }
       
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"root=%s\n\n", lc->root.data);        
    
	return NIGHT_OK;
}

int
night_http_error_page(night_conf_t *cf, night_command_t *cmd, void *conf)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function:\t" "night_http_error_page\n\n");
	
	night_http_core_loc_conf_t 	*lc;
	night_str_t					*value;
	int 						n;
	int							i;
	night_str_t 				uri;
	night_http_error_page_t		*error;
	
	lc = (night_http_core_loc_conf_t*) conf;
	value = cf->args.elts;
	n = cf->args.nelts;
	uri = value[n - 1];
	
	for (i = 1; i < n - 1 ; i++)
	{
		error = night_array_push(&lc->error_pages);
		if (error == NULL)
		{
			dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(error_log_fd,"night_array_push failed while night_http_error_page\n\n");
			
			return NIGHT_ERROR;
		}
		
		error->status = night_atoi(value[i].data, value[i].len);
		if (error->status == NIGHT_ERROR)
		{
			dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(error_log_fd,"error_page configuration value is invalid at %ld\n\n",cf->conf_file->line);
			
			return NIGHT_ERROR;
		}
		
		if(error->status == 499)
		{
			dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(error_log_fd,"error_page configuration value %d is invalid at line %ld\n\n", 
					error->status,cf->conf_file->line);
			
			return NIGHT_ERROR;
		}

        if (error->status < 300 || error->status > 599) 
        {
        	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            dprintf(error_log_fd,"error_page configuration value %d is invalid at line %ld\n\n", 
            		error->status,cf->conf_file->line);
            
            return NIGHT_ERROR;
        }
        
        error->value.data = night_pmalloc(night_cycle->pool, uri.len + 1);
        strcat(error->value.data, uri.data);
        error->value.len = uri.len;
        
        dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(trace_file_fd,"error->status=%d\n", error->status);
        dprintf(trace_file_fd,"error->value.len=%ld\n", error->value.len);
        dprintf(trace_file_fd,"error->value.data=%s\n\n", error->value.data);
	}
	
	return NIGHT_OK;
}

// 该函数在请求被解析完成后（例如请求行、请求头已读取并解析），用于初始化请求的处理状态，并启动 HTTP 处理阶段（phases）的执行
void
night_http_handler(night_http_request_t *r)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_http_handler\n\n");
	
	night_http_core_main_conf_t  			*cmcf;
	
	// 判断当前请求是否为非内部请求（即来自客户端的原始请求）
	// r->internal 为 1 表示这是一个内部重定向、子请求或 error_page 跳转等内部生成的请求。
	if (!r->internal) 
	{
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "if (!r->internal)\n\n");
		
		// 根据客户端在 Connection 请求头中指定的连接类型，决定是否启用 keepalive
		// HTTP/1.1 默认 keepalive，HTTP/1.0 默认关闭，但可通过 Connection: keep-alive 启用。
		switch (r->headers_in.connection_type) 
		{
			// 0：未显式指定（依赖 HTTP 版本）
			// 如果是 HTTP/1.1 或更高（> NIGHT_HTTP_VERSION_10），则默认启用 keepalive（r->keepalive = 1）
			// 否则（HTTP/1.0），不启用 keepalive（r->keepalive = 0）
			case 0:
				dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
				dprintf(trace_file_fd, "case 0:\n\n");
				
            	r->keepalive = (r->http_version > NIGHT_HTTP_VERSION_10);
            	break;

        	case NIGHT_HTTP_CONNECTION_CLOSE:
				dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
				dprintf(trace_file_fd, "case NIGHT_HTTP_CONNECTION_CLOSE:\n\n");
				
            	r->keepalive = 0;
            	break;

        	case NIGHT_HTTP_CONNECTION_KEEP_ALIVE:
				dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
				dprintf(trace_file_fd, "case NIGHT_HTTP_CONNECTION_KEEP_ALIVE:\n\n");
				
            	r->keepalive = 1;
            	break;
		}
		
		// 设置是否启用 lingering close（延迟关闭）。
		// 如果请求体存在（content_length_n > 0）或使用分块传输编码（chunked），则设置 lingering_close = 1
		// lingering_close 表示即使响应已发送完毕， 仍会尝试读取并丢弃客户端可能继续发送的请求体数据（防止客户端因未读完响应而重传）
		// 这是一种防御性机制，避免 TCP RST 导致客户端异常。
		r->lingering_close = (r->headers_in.content_length_n > 0 || r->headers_in.chunked);
		// 将请求的 phase_handler 索引重置为 0
        r->phase_handler = 0;
	}
	// 处理 内部请求（r->internal == 1）的情况
	else 
	{
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "else\tr->internal\n\n");
    	
        cmcf = r->main_conf[night_http_core_module.ctx_index];
		// 为内部请求设置起始处理阶段为 server rewrite 阶段
		// 内部请求（如 error_page 重定向、内部跳转）不需要重新执行 post-read 等早期阶段
		// 通常从 server_rewrite 阶段开始，以应用 server 块中的 rewrite 规则。
		//server_rewrite_index 是 phase engine 中 server rewrite 阶段的起始索引
        r->phase_handler = cmcf->phase_engine.server_rewrite_index;
        
        dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(trace_file_fd, "r->phase_handler=%d\n\n", r->phase_handler);
    }
    
    // 标记当前请求的 location 配置是有效的
    r->valid_location = 1;
    
    // 设置请求的写事件处理器为 night_http_core_run_phases
    // 意味着一旦可写，就继续执行 HTTP 处理阶段
	//r->write_event_handler = night_http_core_run_phases;
	// 立即启动 HTTP 处理阶段的执行
    night_http_core_run_phases(r);
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "night_http_core_run_phases(r) completed\n\n");
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "function night_http_handler:\treturn\n\n");
}

// 这是  HTTP 请求处理的核心函数之一，用于驱动 HTTP 请求在各个处理阶段（phases）中依次执行
// 用于启动并执行请求处理各阶段（如 rewrite、access、content 等）的主调度函数。
// 每个 HTTP 请求在进入处理流程后，都会调用此函数来依次执行配置好的阶段处理器
// 总结：函数整体逻辑
// 获取 HTTP 核心模块的主配置，从中拿到预编译的阶段处理器数组。
// 从 r->phase_handler 指定的阶段开始，依次调用每个阶段的 checker。
// 每个 checker 负责执行该阶段的实际 handler，并决定下一步执行哪个阶段（通过修改 r->phase_handler）。
// 如果某个 checker 返回 NGX_OK（通常意味着 content 阶段已成功生成响应），则立即终止阶段执行。
// 整个过程实现了 Nginx 高度模块化、可扩展的请求处理流水线。
void
night_http_core_run_phases(night_http_request_t *r)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "night_http_core_run_phases\n\n");
	
	int			                   	rc;
    night_http_phase_handler_t   	*ph;
    night_http_core_main_conf_t  	*cmcf;
    
    // 获取阶段处理器数组
    // cmcf->phase_engine 是 night_http_phase_engine_t 类型，其中 handlers 是一个 night_http_phase_handler_t 数组
	// 这个数组按处理顺序排列了所有阶段的 handler（包括 rewrite、access、content 等），每个 handler 包含一个 checker 函数指针和一个 handler 函数指针
	// checker 是实际被调用的“调度器”，它决定如何调用 handler 并处理返回值。
    cmcf = r->main_conf[night_http_core_module.ctx_index];
	ph = cmcf->phase_engine.handlers;
	
	// 开始一个循环，条件是当前阶段的 checker 函数指针不为 NULL
	// r->phase_handler 是请求结构中的一个整型字段，表示当前应执行的阶段在 handlers 数组中的索引
	// 每个阶段的 checker 负责调用实际的 handler，并根据返回值决定下一步（继续、跳转、终止等）
	// 逻辑：只要还有有效的 checker（即未执行完所有阶段），就继续循环
	while (ph[r->phase_handler].checker)
	{
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "r->phase_handler=%d\n\n", r->phase_handler);
		// 调用当前阶段的 checker 函数，并传入请求 r 和当前阶段处理器的地址
		// checker 并不是用户直接编写的 handler，而是 Nginx 为每个阶段提供的“调度器”
		// checker 的职责包括：
		// 调用实际的 handler；
		// 处理返回值（如 NGX_DECLINED 表示继续下一个 handler，NGX_OK 表示阶段完成）；
		// 更新 r->phase_handler（可能跳过某些阶段）。
		// 返回值 rc：通常 checker 自身不直接返回 NGX_OK 给外层，而是通过修改 r->phase_handler 来控制流程。
		// 但某些 checker（如 content phase）在成功处理后会返回 NGX_OK 表示请求已处理完毕。

        rc = ph[r->phase_handler].checker(r, &ph[r->phase_handler]);
        
        dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(trace_file_fd, "rc(%d) = ph[r->phase_handler].checker\n\n", rc);
        
		// 在大多数阶段，checker 不会返回 NGX_OK，而是通过修改 r->phase_handler 让循环继续。
		// 唯一常见返回 NGX_OK 的是 content 阶段的 checker。
		// 当 content handler 成功生成响应（如返回静态文件），其 checker 会返回 NGX_OK，表示“请求已处理完成，无需继续后续阶段”。
		// 因此，一旦收到 NGX_OK，说明响应已生成，可以安全返回。
        if (rc == NIGHT_OK)
        {
        	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "if (rc == NIGHT_OK)\nfunction night_http_core_run_phases:\treturn;\n\n");
        	
            return;
        }
    }
}

//  HTTP 请求处理流程中“rewrite 阶段”的核心处理函数
int
night_http_core_rewrite_phase(night_http_request_t *r, night_http_phase_handler_t *ph)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_http_core_rewrite_phase\n\n");
	
	int  				rc;
	
	// 指向某个模块在 rewrite 阶段注册的处理函数
	rc = ph->handler(r);
	
	// NGX_DECLINED 表示当前 handler 不处理该请求，希望由下一个 handler 继续处理
	if (rc == NIGHT_DECLINED) 
	{
        r->phase_handler++;
        
        dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(trace_file_fd, "return NIGHT_AGAIN;\n\n");
        
        // 返回 NGX_AGAIN：告诉 nginx 的主循环“请再次调用当前阶段”（因为 phase_handler 已更新，下一次会执行下一个 handler）
        return NIGHT_AGAIN;
    }

	// NGX_DONE 表示 handler 已经完全处理了请求，且不需要 nginx 做任何后续操作（例如，已经发送了响应并关闭了连接）
	// 此时直接返回 NGX_OK，表示“阶段处理成功完成”，nginx 不会再调用 ngx_http_finalize_request。
    if (rc == NIGHT_DONE) 
    {
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(trace_file_fd, "return NIGHT_OK;\n\n");
        
        return NIGHT_OK;
    }
	
	// NGX_OK, NGX_AGAIN, NGX_ERROR, NGX_HTTP_...  
	// 如果 rc 不是 NGX_DECLINED 或 NGX_DONE，那么它可能是：
	// 结束当前请求的处理流程，并发送响应
    night_http_finalize_request(r, rc);
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "ngx_http_finalize_request(r, rc);\n");
	dprintf(trace_file_fd, "return NGX_OK;\n\n");
	
	return NIGHT_OK;
}

/* 
它是 Nginx HTTP 请求处理 11 个阶段（phases）中的 NGX_HTTP_FIND_CONFIG_PHASE 阶段的处理函数。
该阶段的主要职责是：
根据当前请求的 URI 查找最匹配的 location 配置块（location block）
更新请求的上下文（如 location 配置、变量等）
检查请求体大小是否超出限制
如果匹配的是带重定向的 location（如 = 精确匹配但需要重定向），则返回 301 重定向
否则继续下一个处理阶段
*/
int
night_http_core_find_config_phase(night_http_request_t *r, night_http_phase_handler_t *ph)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_http_core_find_config_phase\n\n");
	
	int 							rc;
	night_http_core_loc_conf_t  	*clcf;
	
	/*
	content_handler：用于 content phase 的自定义处理器（如 proxy、fastcgi 等）。此处重置为 NULL，因为 location 可能已变，旧 handler 无效。
	uri_changed：标记 URI 是否被 rewrite 模块修改过。此处设为 0，表示尚未改变（此阶段不处理 rewrite，只是查找配置）。
	*/
	r->content_handler = NULL;
    r->uri_changed = 0;
    
    /*
    	调用核心函数 ngx_http_core_find_location，根据 r->uri 在 server 块的 location 列表中查找最匹配的 location。
	返回值：
	NGX_OK：找到匹配 location。
	NGX_DONE：需要 301 重定向（例如：请求 /foo，但配置了 /foo/，Nginx 会重定向到带 / 的版本）。
	NGX_ERROR：出错。
    */
    rc = night_http_core_find_location(r);
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "%d = ngx_http_core_find_location(r);\n\n", rc);
	
    // 错误处理
    /*
    	如果查找 location 出错（如内存分配失败），则：
	调用 ngx_http_finalize_request 终止请求，返回 500 Internal Server Error。
	返回 NGX_OK 表示“阶段已处理完毕，无需继续”（注意：这里 NGX_OK 是阶段返回值，不是错误）。
 	注意：在 Nginx 阶段处理中： 

	NGX_OK 表示“本阶段完成，不再继续本阶段后续 handler”。
	NGX_AGAIN 表示“继续下一个阶段”。
	NGX_ERROR 通常用于内部错误。
    */
	if (rc == NIGHT_ERROR) 
	{
        night_http_finalize_request(r, NIGHT_HTTP_INTERNAL_SERVER_ERROR);
        return NIGHT_OK;
    }
    
    // 从请求 r 中获取当前匹配到的 core 模块的 location 配置
    clcf = (r)->loc_conf[night_http_core_module.ctx_index];
    
    /*
    r->internal：表示该请求是否是 内部跳转（如 error_page、try_files、X-Accel-Redirect 触发的）。
	clcf->internal：表示该 location 被标记为 internal（只能内部访问）。
	如果 外部请求（非 internal）访问了 internal location，则返回 404 Not Found（注意：不是 403，这是 Nginx 的安全设计，避免暴露内部路径存在）。
    */
	if (!r->internal && clcf->internal) 
	{
        night_http_finalize_request(r, NIGHT_HTTP_NOT_FOUND);
        return NIGHT_OK;
    }
    
    // 将 clcf 中的配置（如 root、alias、index、client_max_body_size 等）应用到请求 r 的运行时上下文中
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "night_http_update_location_config(r);\n\n");
	
    night_http_update_location_config(r);
 	
 	//NGX_DONE 表示：URI 需要规范化并重定向
 	if (rc == NIGHT_DONE) 
 	{
 	}
    
    // 阶段索引加 1，指向下一个处理阶段
	r->phase_handler++;
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "r->phase_handler=%d\n", r->phase_handler);
	dprintf(trace_file_fd, "return NIGHT_AGAIN;\n\n");
	
	//返回 NGX_AGAIN：表示“继续执行后续阶段”
    return NIGHT_AGAIN;
}

int
night_http_core_post_rewrite_phase(night_http_request_t *r, night_http_phase_handler_t *ph)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" "function:\t" "night_http_core_post_rewrite_phase\n\n" , __FILE__, __LINE__);
	
	/*
	检查在 rewrite 阶段（前一阶段）是否发生了 URI 的变更
	如果 没有变更（r->uri_changed == 0）:
	将 phase_handler 自增 1，表示进入下一个处理阶段；
	返回 NGX_AGAIN，通知 nginx 继续执行下一个阶段。
	*/
	if (!r->uri_changed) 
	{
        r->phase_handler++;
        
		dprintf(trace_file_fd, 	"FILE=%s:LINE=%d\n" "if (!r->uri_changed)\n" "r->phase_handler=%d\n" "return NGX_AGAIN;\n\n",
		 __FILE__, __LINE__ , r->phase_handler);
		 
        return NIGHT_AGAIN;
    }
	
	return NIGHT_OK;
}    

/*
HTTP 请求处理阶段中的 access 阶段（访问控制阶段）,
该函数负责调用 access 阶段注册的 handler，并根据返回值决定是否继续执行后续 handler、跳过阶段，或直接结束请求
*/
int
night_http_core_access_phase(night_http_request_t *r, night_http_phase_handler_t *ph)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_http_core_access_phase\n\n");
	
	int								rc;
	
	/*
	如果当前请求 r 不是主请求（即是一个子请求），则 跳过整个 access 阶段
	子请求通常由内部重定向、SSI、error_page 等机制生成，其权限应由主请求控制，避免重复检查
	直接将 phase_handler 设置为 ph->next（即跳过当前阶段所有 handler）。
	返回 NGX_AGAIN 表示继续执行下一个阶段。
	*/
	if (r != r->main) 
	{
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" "if (r != r->main)\n\n", __FILE__, __LINE__);
		
        r->phase_handler = ph->next;
        return NIGHT_AGAIN;
    }
    
    // 调用当前 handler
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" "rc = ph->handler(r);\n\n", __FILE__, __LINE__);
	
    rc = ph->handler(r);
    
    /*
    NGX_DECLINED 表示当前 handler 不处理此请求（例如配置未匹配），应继续执行下一个 access handler
	phase_handler++：移动到下一个 handler。
	返回 NGX_AGAIN：让 Nginx 继续执行下一个 handler（仍在 access 阶段）。
    */
	if (rc == NIGHT_DECLINED) 
	{
        r->phase_handler++;
        
		dprintf(trace_file_fd, 	"FILE=%s:LINE=%d\n" "if (rc == NIGHT_DECLINED)\nr->phase_handler=%d\nreturn NIGHT_AGAIN;\n\n",
								__FILE__, __LINE__, r->phase_handler);
        						 
        return NIGHT_AGAIN;
    }
    
	return NIGHT_OK;
}

int
night_http_core_post_access_phase(night_http_request_t *r, night_http_phase_handler_t *ph)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_http_core_post_access_phase\n\n");
	
	r->phase_handler++;
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" "r->phase_handler=%d\nreturn NIGHT_AGAIN;\n\n", __FILE__, __LINE__, r->phase_handler);
    
    return NIGHT_AGAIN;
}

// 该函数是 Nginx HTTP 请求处理流程中 content phase（内容生成阶段） 的核心处理函数
int
night_http_core_content_phase(night_http_request_t *r, night_http_phase_handler_t *ph)    
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_http_core_content_phase\n\n");
	
	int					rc;
/*
如果请求 r 已经被某个模块（如 location 块中配置了 index 或 fastcgi_pass 等）设置了 content_handler，
则直接调用该 handler（r->content_handler(r)）来生成响应内容。
r->write_event_handler = ngx_http_request_empty_handler;：
将写事件处理器设为空函数，防止在 content handler 执行期间触发其他写事件逻辑（避免重复处理）。
ngx_http_finalize_request(r, ...)：
根据 handler 的返回值（如 NGX_OK、NGX_ERROR）完成请求（可能发送响应、关闭连接等）。
返回 NGX_OK：表示该 phase 已处理完毕，无需继续。
这是性能优化：一旦有明确的 content handler，就跳过后续的 phase 遍历。 
*/
	if (r->content_handler) 
	{
        r->write_event_handler = night_http_request_empty_handler;
        night_http_finalize_request(r, r->content_handler(r));
        return NIGHT_OK;
    }
    
    // ph->handler：是当前 content phase 中要执行的 handler 函数（例如 ngx_http_static_handler、ngx_http_autoindex_handler 等）
	// 执行该 handler，并获取返回值 rc
   	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" "rc = ph->handler(r);\n\n" , __FILE__, __LINE__);
	
    rc = ph->handler(r); 
  
/*
 处理 handler 返回值  
如果 handler 没有返回 NGX_DECLINED，说明它“处理了”该请求（成功或失败）。
NGX_OK：成功生成内容。
NGX_ERROR / NGX_HTTP_XXX：错误或 HTTP 状态码。
调用 ngx_http_finalize_request 完成请求。
返回 NGX_OK 表示 content phase 结束。
*/
    if (rc != NIGHT_DECLINED) 
    {
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "if (rc != NIGHT_DECLINED) {\nnight_http_finalize_request(r, rc);\n\n");
    	
        night_http_finalize_request(r, rc);
        
        dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(trace_file_fd, "night_http_finalize_request(r, rc) completed\n\n");
        
        dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(trace_file_fd, "function night_http_core_content_phase:\treturn NIGHT_OK;\n\n");
        
        return NIGHT_OK;
    }
    
// 若返回 NGX_DECLINED：表示当前 handler 无法处理，尝试下一个
// 移动到下一个 phase handler
// 检查下一个 handler 是否有效（ph->checker 非空表示还有后续 handler）。
//如果有：
//r->phase_handler++：更新请求的当前 handler 索引。
//返回 NGX_AGAIN：通知 Nginx 框架重新调用当前 phase（即 content phase），但使用下一个 handler。

    ph++;

    if (ph->checker) 
    {
        r->phase_handler++;
        
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "if (ph->checker)\n");
    	dprintf(trace_file_fd, "r->phase_handler=%d\n", r->phase_handler);
    	dprintf(trace_file_fd, "return NIGHT_AGAIN;\n\n");
    	
        return NIGHT_AGAIN;
    }
    
	return NIGHT_OK;
}    

/*
处理 HTTP 请求处理流程中的某些通用阶段，比如 post read（读取请求头后）和 pre-access（访问控制前）阶段
*/
int
night_http_core_generic_phase(night_http_request_t *r, night_http_phase_handler_t *ph)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" "function:\t" "night_http_core_generic_phase\n\n" , __FILE__, __LINE__);
	
	int 						rc;
	
	// 调用当前阶段的 handler 函数
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" "rc = ph->handler(r);\n\n" , __FILE__, __LINE__);
	
	rc = ph->handler(r);
	
	// 处理 handler 返回 NGX_OK 的情况
	// 表示当前 handler 成功完成其任务，并且希望跳过后续某些阶段（通常是因为它已经设置了下一个要执行的阶段）
	if (rc == NIGHT_OK)
	{
		/*
		ph->next 是在 nginx 初始化阶段（如 ngx_http_init_phase_handlers）预计算好的“下一个应执行的 handler 索引”。
		将 r->phase_handler 设置为 ph->next，表示下一次调用时从那个位置继续。
		返回 NGX_AGAIN：注意，这里的 NGX_AGAIN 并不是“重试”，而是告诉 nginx 的主循环：“我还没处理完请求，请继续调用下一个阶段”
		*/
		r->phase_handler = ph->next;
		return NIGHT_AGAIN;
	}
	
	/* 
	处理 handler 返回 NGX_DECLINED 的情况
	NGX_DECLINED 表示“本 handler 不处理此请求，交给下一个 handler”
	r->phase_handler++：将阶段索引加 1，即顺序执行 handler 数组中的下一个 handler
	同样返回 NGX_AGAIN，表示继续处理流程
	*/
	if (rc == NIGHT_DECLINED) 
	{
		r->phase_handler++;
		
		dprintf(trace_file_fd, 	"FILE=%s:LINE=%d\n" "if (rc == NIGHT_DECLINED)\n" "r->phase_handler=%d\n" "return NIGHT_AGAIN;\n\n" ,
							 	__FILE__, __LINE__, r->phase_handler);
		
		return NIGHT_AGAIN;
	}
	
	return NIGHT_OK;
}

// 用于在 HTTP 请求处理过程中查找匹配的 location 配置块
int
night_http_core_find_location(night_http_request_t *r)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_http_core_find_location\n\n");
	
	int								rc;
	night_http_core_loc_conf_t		*pclcf;
	
	pclcf = r->loc_conf[night_http_core_module.ctx_index];
	
	// 在静态 location 列表中查找匹配项
	rc = night_http_core_find_static_location(r, pclcf->static_locations_tree);
	
	// 如果静态 location 查找返回 NGX_AGAIN，说明找到了一个前缀匹配的 location，且该 location 内部定义了嵌套的子 location，需要继续在子 location 中查找
	if (rc == NIGHT_AGAIN) 
	{
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "if (rc == NIGHT_AGAIN)\n\n");
    	
    	// 作用：递归调用自身，在当前匹配到的 location 的子 location 中继续查找
		// look up nested locations

        rc = night_http_core_find_location(r);
    	
	}
	
	/*
	如果递归查找或静态查找的结果是 NGX_OK（精确匹配）或 NGX_DONE（前缀匹配完成，无需进一步处理），则直接返回。
	说明：
	NGX_DONE 通常表示找到了最长前缀匹配，且没有嵌套或不需要继续处理。
	此时无需检查正则 location
	*/
	if (rc == NIGHT_OK || rc == NIGHT_DONE) 
	{
        return rc;
    }
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "rc=%d\n\n", rc);
	
	return rc;
}

// 在静态 location 树中查找与当前请求 URI 匹配的 location 配置块
int
night_http_core_find_static_location(night_http_request_t *r, night_http_location_tree_node_t *node)
{
   	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_http_core_find_static_location\n\n");
	
	char		*uri;
    size_t      len;
    size_t		n;
    int			rc;
    int			rv;
    
	len = r->uri.len;
    uri = r->uri.data;
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "uri=");
    write(trace_file_fd, r->uri.data, r->uri.len);
    dprintf(trace_file_fd, "\n\n");
    
    // 初始化返回值为 NGX_DECLINED，表示“未找到匹配”
    rv = NIGHT_DECLINED;
    
    // 开始无限循环，遍历 location 树，直到找到匹配或 node == NULL
    for ( ;; ) 
    {
		if (node == NULL) 
		{
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "if (node == NULL)\n");
        	dprintf(trace_file_fd, "rv=%d\n\n", rv);
        	
            return rv;
        }
        
        // 计算要比较的字节数 n：取 URI 长度和节点路径长度的较小值
		// 例如：URI 是 /static/a（len=10），节点是 /static（len=7）→ n = 7
        n = (len <= (size_t) node->len) ? len : node->len;
        
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "node->len=%ld\n", node->len);
		dprintf(trace_file_fd, "n=%ld\n", n);
		dprintf(trace_file_fd, "node->name=%s\n\n", node->name);
		
        /*
        		使用 ngx_filename_cmp 比较 URI 前 n 字节与节点路径 node->name。
		注意：ngx_filename_cmp 是大小写敏感的字节比较（类似 memcmp，但处理平台差异）
		返回值：
		< 0：URI 字典序小于节点路径 → 应走左子树。
		> 0：URI 字典序大于节点路径 → 应走右子树。
		== 0：前 n 字节相同 → 可能匹配。
        */
        rc = night_filename_cmp(uri, node->name, n);
        
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "rc=%d\n\n", rc);
		
		/*
        		如果前 n 字节不相等（rc != 0）：
		根据字典序决定向左（rc < 0）或向右（rc > 0）继续搜索。
		continue 跳过后续逻辑，进入下一轮循环。
        */
		if (rc != 0) 
		{
            node = (rc < 0) ? node->left : node->right;

            continue;
        }
        
		// 前 n 字节相等（rc == 0）
		/*
		如果 URI 比当前节点路径更长（例如 URI=/static/a，节点=/static）：
		说明 URI 以该节点路径为前缀
		*/
		if (len > (size_t) node->len) 
		{
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "if (len > (size_t) node->len)\n\n");
        	
			// 检查该节点是否有前缀匹配（inclusive）配置（即普通 location /xxx）
			if (node->inclusive) 
			{
				dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
				dprintf(trace_file_fd, "if (node->inclusive)\n\n");
				
				/*
				如果有前缀匹配：
				将请求的 loc_conf（location 配置指针）设为该 inclusive location 的配置。
				设置返回值为 NGX_AGAIN，表示“找到了前缀匹配，但可能还有更精确的匹配”。
				NGX_AGAIN 在 Nginx 中常用于“部分匹配，继续处理”。 
				*/
                r->loc_conf = node->inclusive->loc_conf;
                rv = NIGHT_AGAIN;
				
				// 跳转到该节点的子树
                node = node->tree;
                // URI 指针前进 n 字节（跳过已匹配的前缀）
                uri += n;
                // 更新剩余长度
                len -= n;
				// 继续循环，匹配子路径
                continue;
			}  
                /*
                			 举例： 
				URI: /static/images/logo.png
				当前节点: /static（inclusive）
				匹配后：uri 指向 images/logo.png，进入 /static 的 tree 子树继续匹配
                */
                
			/*
				如果 len > node->len 但 没有 inclusive 配置（只有 exact）：
				说明该节点只支持精确匹配（如 location = /static），而 URI 更长（如 /static/a），不匹配。
				但字典序上 URI ≥ 节点路径（因为前 n 字节相等），所以应向右子树搜索更大的路径。
				例如：节点是 /stat（exact），URI 是 /static → 虽然前 4 字节相同，但 URI 更长，且节点无 inclusive，所以去右子树找 /static。
			*/
			 
			//exact only 
			
			node = node->right;

			continue; 
            
		}
		
		// URI 长度等于节点路径长度 → 可能是精确匹配
		if (len == (size_t) node->len) 
		{
			
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "if (len == (size_t) node->len)\n\n");
        	
        	/*
        				如果节点有 exact 配置（即 location = /xxx）：
				设置 loc_conf 为 exact 配置。
				立即返回 NGX_OK，因为精确匹配优先级最高，无需继续查找。
        	*/
            if (node->exact) 
            {
            
				dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            	dprintf(trace_file_fd, "if (node->exact)\n");
            	dprintf(trace_file_fd, "return NGX_OK;\n\n");
            	
                r->loc_conf = node->exact->loc_conf;
                return NIGHT_OK;

            } 
            /*
           		 	如果没有 exact，但有 inclusive（即普通前缀匹配，且 URI 完全等于该前缀）：
				设置配置，并返回 NGX_AGAIN。
				注意：虽然 URI 完全匹配，但因为是前缀匹配（非 =），仍可能被后续正则 location 覆盖，所以返回 AGAIN 而非 OK。
				Nginx 匹配顺序： 
				精确匹配（=） → 返回 OK，结束。
				最长前缀匹配 → 记录，继续。
				正则匹配（按顺序） → 若匹配则覆盖。
				所以此处返回 AGAIN 表示“这是一个候选，但还要看正则”。
            */
            else 
            {
            
				dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            	dprintf(trace_file_fd, "else\n");
            	dprintf(trace_file_fd, "return NGX_AGAIN;\n\n");
            	
                r->loc_conf = node->inclusive->loc_conf;
                return NIGHT_AGAIN;
            }
        }
        
        // len < node->len
        
        node = node->left;
        
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(trace_file_fd, "node = node->left;\n\n");
    }
}

// 根据当前请求所匹配的 location 配置更新请求上下文中的运行时参数
void
night_http_update_location_config(night_http_request_t *r)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_http_update_location_config\n\n");
	
	night_http_core_loc_conf_t  			*clcf;
	
	// 获取 location 配置
	clcf = r->loc_conf[night_http_core_module.ctx_index];
	
	// 处理 keepalive 连接的各种限制条件
	// 仅当当前请求期望保持连接时才检查
 	if (r->keepalive) 
 	{
 	}
    
	return;
}

/*
其作用是将 HTTP 请求的 URI 映射为服务器本地文件系统中的实际路径（即构建完整的文件路径）
这是 Nginx 处理静态文件请求（如 location /static { root /var/www; }）时的核心逻辑之一
返回值：指向生成路径中 URI 部分起始位置的指针（即路径中去掉 root 后的部分的起始地址），便于后续处理（如判断是否为目录）。
参数说明：
r：当前 HTTP 请求结构体。
path：输出参数，用于返回构建好的完整文件路径（字符串）
root_length：输出参数，返回 root 路径部分的长度（不包括 URI 部分）
reserved：预留空间大小，通常用于后续追加内容（如 /index.html）
*/
char*
night_http_map_uri_to_path(night_http_request_t *r, night_str_t *path, size_t *root_length, size_t reserved)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_http_map_uri_to_path\n\n");
	
	char                    		*last;
	night_http_core_loc_conf_t  	*clcf;
	
	// 获取 location 配置
    clcf = r->loc_conf[night_http_core_module.ctx_index];
    
    // 处理静态 root（无变量）
	// clcf->root_lengths 是一个数组，用于存储包含变量（如 $host）的 root 或 alias 指令的脚本长度信息
	// 如果为 NULL，说明 root 是静态字符串（不含变量），走简单路径
	if (clcf->root_lengths == NULL) 
	{
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "if (clcf->root_lengths == NULL)\n\n");
    	
		// 直接将 root 字符串长度赋给输出参数 root_length
        *root_length = clcf->root.len;
        
		// 计算总路径长度
		// 总长度 = root 长度 + 预留空间 + URI 长度
        path->len = clcf->root.len + reserved + r->uri.len;
        
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(trace_file_fd, 	"path->len(%ld) = clcf->root.len(%ld) + reserved(%ld) + r->uri.len(%ld);\n\n",
        						 path->len, clcf->root.len, reserved, r->uri.len);
        						 
		// 分配内存
        path->data = night_pmalloc(r->pool, path->len);
        if (path->data == NULL) 
        {
            return NULL;
        }
        
		// 拷贝 root 路径
        last = memcpy(path->data, clcf->root.data, clcf->root.len);
        last += clcf->root.len;
        
        dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(trace_file_fd, "path->data=%s\n\n", path->data);			 
	}

	// 追加 URI 到路径
    last = memcpy(last, r->uri.data, r->uri.len);
    last += r->uri.len;
    *last = '\0';
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "path->data=%s\n\n", path->data);

	return last;
}

//用于在 HTTP 请求处理过程中执行内部重定向（internal redirect）。内部重定向不同于客户端重定向（如 301/302），它完全在服务器内部完成，客户端无感知
int
night_http_internal_redirect(night_http_request_t *r, night_str_t *uri, night_str_t *args)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_http_internal_redirect\n\n");
	
	night_http_core_srv_conf_t  *cscf;
	size_t						n;
	
	//r->uri_changes 是一个计数器，初始值通常为 NGX_HTTP_MAX_URI_CHANGES（默认为 10）。
	//每次内部重定向时减 1，防止无限重定向循环（如 rewrite 规则错误导致死循环）。
	//如果减到 0，说明重定向次数超限。
    r->uri_changes--;
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "r->uri_changes=%d\n\n", r->uri_changes);
	
	if (r->uri_changes == 0) 
	{
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "if (r->uri_changes == 0)\n"); 
		dprintf(trace_file_fd, "rewrite or internal redirection cycle\n" "while internally redirecting to \"%s\"", uri->data);
		
		// 增加主请求的引用计数
        r->main->count++;
        // 终止当前请求，返回 500 Internal Server Error
        night_http_finalize_request(r, NIGHT_HTTP_INTERNAL_SERVER_ERROR);
        
        //返回 NGX_DONE，表示该请求已由本函数处理完毕，调用者无需再处理。
		//NGX_DONE 是 Nginx 中常见的返回值，表示“操作已完成，无需进一步动作”。
        return NIGHT_DONE;
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "r->uri=");
	write(trace_file_fd, r->uri.data, r->uri.len);
    dprintf(trace_file_fd, "\n\n");
    
	// 将当前请求的 URI 更新为目标 URI
    r->uri = *uri;
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "r->uri=%s\n\n", r->uri.data);
    
	//设置请求的查询参数（即 ? 后面的部分）
	//如果 args 非空，则复制其内容到 r->args
	//否则，将 r->args 置为空字符串
    if (args) 
    {
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "args=");
    	n = write(trace_file_fd, r->args.data, r->args.len);
    	dprintf(trace_file_fd, "\nn=%ld\n\n", n);
    	
        r->args = *args;

    } else {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "args is null\n\n");
    	
        night_str_null(&r->args);
    }
    
	// 重新解析文件扩展名
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "night_http_set_exten(r);\n\n");
	
    night_http_set_exten(r);
    
	//清空所有 HTTP 模块的上下文（context）：
	//r->ctx 是一个指针数组，每个模块在处理请求时可存储自己的上下文数据。
	//内部重定向后，之前的模块状态已无效，必须清零，避免旧状态干扰新 URI 的处理。
    night_memzero(r->ctx, sizeof(void *) * night_http_modules_n);
    
	// 获取当前请求对应的 server 块的核心配置
	cscf = r->srv_conf[night_http_core_module.ctx_index];
    
    //重置 location 配置为 server 的默认 location 配置：
    r->loc_conf = cscf->ctx->loc_conf;

	// 根据当前 r->uri 重新查找并应用匹配的 location 配置
    night_http_update_location_config(r);
    
	// 标记该请求为 内部请求
    r->internal = 1;
    
    // 标记原始 URI（r->unparsed_uri）不再有效
    r->valid_unparsed_uri = 0;
    
    //再次增加主请求的引用计数
    r->main->count++;
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "r->main->count=%d\n\n", r->main->count);
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "night_http_handler(r);\n\n");
	
    night_http_handler(r);
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "night_http_internal_redirect return NIGHT_DONE;\n\n");
    
	return NIGHT_DONE;
}

// 该函数用于从 HTTP 请求的 URI 中提取文件扩展名（extension），并将其保存在请求结构体 r 的 exten 字段中。
void
night_http_set_exten(night_http_request_t *r)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_http_set_exten\n\n");
	
    size_t		i;
	
	//初始化扩展名为空
    night_str_null(&r->exten);

	//for 循环开始，从 URI 末尾向前遍历
	// i > 1 → 至少要保留前两个字符（索引 0 和 1），如 /a.b 是合法的
    for (i = r->uri.len - 1; i > 1; i--) {
    	//判断当前位置 i 是否是一个有效的扩展名分隔符（即 .），并且它前面不是路径分隔符 /
        if (r->uri.data[i] == '.' && r->uri.data[i - 1] != '/') {

			// 一旦找到有效的 .，就设置 r->exten 指向扩展名部分
            r->exten.len = r->uri.len - i - 1;
            r->exten.data = &r->uri.data[i + 1];
			
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(trace_file_fd, "r->exten.data=%s\n\n", r->exten.data);
			
            return;

        } else if (r->uri.data[i] == '/') {
        	
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(trace_file_fd, "else if (r->uri.data[i] == '/')\n");
			dprintf(trace_file_fd, "r->exten.data=%s\n\n", r->exten.data);
			
        	// 如果在从后往前遍历过程中遇到了 /，说明已经进入路径的目录部分，后面不可能再有文件扩展名了，直接退出。
            return;
        }
    }

    return;
}

/*
这段代码来自 Nginx 1.24.0 源码中的 ngx_http_set_etag 函数，其作用是为 HTTP 响应生成并设置 ETag 响应头。
ETag（Entity Tag）是 HTTP 协议中用于缓存验证的一种机制，通常由服务器根据资源的最后修改时间和内容长度等信息生成一个唯一标识符
*/
int
night_http_set_etag(night_http_request_t *r)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_http_set_etag\n\n");
	
	night_table_elt_t		*etag;
	
	etag = night_list_push(&r->headers_out.headers);
    if (etag == NULL) 
    {
        return NIGHT_ERROR;
    }
    
	/*
	设置 etag->hash = 1，表示该头部字段应被包含在响应头的哈希表中（用于快速查找，比如在 ngx_http_send_header 中）。
	如果 hash 为 0，则该头不会被发送（或不会被索引）。
	*/
    etag->hash = 1;
    etag->next = NULL;
    night_str_set(&etag->key, "ETag");
    
	// 为 ETag 的值（value.data）分配内存。
    etag->value.data = night_pmalloc(r->pool, NIGHT_OFF_T_LEN + NIGHT_TIME_T_LEN + 4);
    if (etag->value.data == NULL) 
    {
        etag->hash = 0;
        return NIGHT_ERROR;
    }
    
	etag->value.len = sprintf(etag->value.data, "\"%lx-%lx\"", r->headers_out.last_modified_time, r->headers_out.content_length_n);
	
	//将生成的 etag 头部指针保存到 r->headers_out.etag 字段中。
	r->headers_out.etag = etag;
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "r->headers_out.etag->value.len=%ld\n", r->headers_out.etag->value.len);
    dprintf(trace_file_fd, "r->headers_out.etag->value.data=%s\n", r->headers_out.etag->value.data);
    dprintf(trace_file_fd, "return NIGHT_OK;\n\n");
    
    return NIGHT_OK;
}
	
int
night_http_set_content_type(night_http_request_t *r)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_http_set_content_type\n\n");
	
	night_http_core_loc_conf_t  	*clcf;
	char							c;
	char*							exten;
	uint64_t						hash;
	uint64_t						i;
	night_str_t                 	*type;

	// 作用：检查响应头中的 Content-Type 是否已经被设置（即长度非零）
	// 逻辑：如果已经设置，直接返回 NGX_OK，避免重复设置
    if (r->headers_out.content_type.len) 
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "if (r->headers_out.content_type.len)\n\n");
    	
        return NIGHT_OK;
    }	
    
    clcf = r->loc_conf[night_http_core_module.ctx_index];
    
	// 检查请求的文件扩展名（r->exten）是否存在
	// 只有存在扩展名时，才尝试根据扩展名查找 MIME 类型
    if (r->exten.len) 
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "if (r->exten.len)\n\n");

        hash = 0;
        
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "r->exten.data=%s\n\n", r->exten.data);
	
		// 遍历扩展名的每一个字符
		// 逐字符处理，用于计算哈希或检测是否包含大写字母
		for (i = 0; i < r->exten.len; i++) 
		{
			c = r->exten.data[i];
			
			//如果是大写字母
			if (c >= 'A' && c <= 'Z') 
			{
                exten = night_pmalloc(r->pool, r->exten.len);
                if (exten == NULL) 
                {
                    return NIGHT_ERROR;
                }

				// 将原扩展名 r->exten.data 转为小写，结果存入 exten
				// 同时计算该小写字符串的哈希值，返回并赋给 hash
                hash = night_hash_strlow(exten, r->exten.data, r->exten.len);

				// 将 r->exten.data 指向新分配的小写扩展名内存
                r->exten.data = exten;

                break;
            }
            
			// 如果当前字符不是大写字母，则将其加入哈希计算
            hash = night_hash(hash, c);
            
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            dprintf(trace_file_fd, "hash=%ld\n\n", hash);
            
		}
		
/*
作用：在 MIME 类型哈希表 clcf->types_hash 中查找对应扩展名的类型。
参数说明：
&clcf->types_hash：哈希表指针。
hash：之前计算的哈希值。
r->exten.data 和 r->exten.len：键（扩展名）及其长度。
返回值：若找到，返回指向 ngx_str_t 的指针（即 MIME 类型字符串）；否则返回 NULL。
意义：这是核心查找逻辑，决定 Content-Type 的值。
*/
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "night_hash_find\n\n");
		
        type = night_hash_find(&clcf->types_hash, hash, r->exten.data, r->exten.len);
        
        dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(trace_file_fd, "type->data=%s\n\n", type->data);
/*
作用：如果找到对应的 MIME 类型：
将 type->len 赋给 content_type_len。
将整个 ngx_str_t 结构体（包含 len 和 data）复制给 content_type。
意义：成功设置 Content-Type，函数返回成功。
*/
        if (type) 
        {
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "if (type)\nreturn NIGHT_OK;\n\n");
        	
            r->headers_out.content_type_len = type->len;
            r->headers_out.content_type = *type;

            return NIGHT_OK;
        }
   } 
   
/*
如果扩展名为空，或未在哈希表中找到匹配项，则使用 location 配置中的默认类型（通常是 "text/plain" 或 "application/octet-stream"）。
意义：保证所有响应都有 Content-Type，符合 HTTP 规范。
*/
    r->headers_out.content_type_len = clcf->default_type.len;
    r->headers_out.content_type = clcf->default_type;
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "return NIGHT_OK;\n\n");
    
    return NIGHT_OK;
}   


/*
该函数用于向客户端发送 HTTP 响应头
*/
int
night_http_send_header(night_http_request_t *r)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_http_send_header\n\n");

	// r->header_sent 是一个标志位，表示响应头是否已经被发送
	// HTTP 协议规定响应头只能发送一次，重复发送会导致协议错误或客户端解析失败
    if (r->header_sent) 
    {
    	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd,"header already sent\n\n");
    	
        return NIGHT_ERROR;
    }

	// 作用：如果请求过程中发生了错误（设置了 err_status），则用该错误状态覆盖正常的响应状态码。
	// r->headers_out.status（即将发送的 HTTP 状态码）设置为 err_status
	// r->headers_out.status_line.len 设为 0，表示“状态行字符串需要重新生成”
    if (r->err_status) 
    {
		dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd,"if (r->err_status)\n\n");
    	
        r->headers_out.status = r->err_status;
        r->headers_out.status_line.len = 0;
    }

	// 调用 header filter 链的入口，开始实际发送响应头
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "return night_http_top_header_filter(r);\n\n");
	
    return night_http_top_header_filter(r);
}
 
/*
用于将响应数据（body）发送给客户端的核心输出过滤器函数。
*/
int
night_http_output_filter(night_http_request_t *r, night_chain_t *in)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_http_output_filter\n\n");
	
    int          		rc;
    night_connection_t  *c;

    c = r->connection;

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "rc = night_http_top_body_filter(r, in);\n\n");

    rc = night_http_top_body_filter(r, in);

    if (rc == NIGHT_ERROR) 
    {
        c->error = 1;
    }

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "return rc(%d);\n\n", rc);
	
    return rc;
} 
