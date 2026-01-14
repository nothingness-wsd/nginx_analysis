#include "night_core.h"

int
night_conf_full_name(night_cycle_t *cycle, night_str_t *name, int conf_prefix)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function:\t" "%s\n\n", __func__);
	
    night_str_t  	*prefix;
    int				rc;

	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"conf_prefix=%d\n\n", conf_prefix);
	
    prefix = conf_prefix ? &cycle->conf_prefix : &cycle->prefix;

    rc = night_get_full_name(cycle->pool, prefix, name);
    
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function %s:\t" "return\n\n", __func__);
	
    return rc;
}
