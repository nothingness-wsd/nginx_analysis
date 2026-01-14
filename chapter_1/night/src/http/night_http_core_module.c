#include "night_core.h"

static int
night_http_core_preconfiguration(night_conf_t *cf)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "%s\n\n", __func__);
	
	int rc;
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "night_http_variables_add_core_vars\n\n");
	
	rc = night_http_variables_add_core_vars(cf);
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function %s\t" "return\n\n", __func__);
	
    return rc;
}

static int
night_http_core_postconfiguration(night_conf_t *cf)
{

}

static void *
night_http_core_create_main_conf(night_conf_t *cf)
{

}

static char *
night_http_core_init_main_conf(night_conf_t *cf, void *conf)
{

}

static void *
night_http_core_create_srv_conf(night_conf_t *cf)
{

}

static char *
night_http_core_merge_srv_conf(night_conf_t *cf, void *parent, void *child)
{

}

static void *
night_http_core_create_loc_conf(night_conf_t *cf)
{

}

static char *
night_http_core_merge_loc_conf(night_conf_t *cf, void *parent, void *child)
{

}

static char *
night_http_core_server(night_conf_t *cf, night_command_t *cmd, void *dummy)
{

}

static night_command_t night_http_core_commands[] = 
{
	{ 
		night_string("server"),
      	NIGHT_HTTP_MAIN_CONF | NIGHT_CONF_BLOCK | NIGHT_CONF_NOARGS,
      	night_http_core_server,
      	0,
      	0,
      	NULL 
	},
      
	night_null_command
};

static night_http_module_ctx_t  night_http_core_module_ctx = 
{
    night_http_core_preconfiguration,        /* preconfiguration */
    night_http_core_postconfiguration,       /* postconfiguration */

    night_http_core_create_main_conf,        /* create main configuration */
    night_http_core_init_main_conf,          /* init main configuration */

    night_http_core_create_srv_conf,         /* create server configuration */
    night_http_core_merge_srv_conf,          /* merge server configuration */

    night_http_core_create_loc_conf,         /* create location configuration */
    night_http_core_merge_loc_conf           /* merge location configuration */
};


night_module_t  night_http_core_module = 
{
    "night_http_core_module",
    NIGHT_MODULE_UNSET_INDEX,
    NIGHT_MODULE_UNSET_INDEX,
    NIGHT_HTTP_MODULE,
    &night_http_core_module_ctx,
    night_http_core_commands
};
