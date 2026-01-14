#include "night_core.h"

static int
night_test_full_name(night_str_t *name)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function:\t" "%s\n\n", __func__);
	
	if (name->data[0] == '/') 
	{
		dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd,"function %s:\t" "return NIGHT_OK\n\n", __func__);
	
        return NIGHT_OK;
    }

	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function %s:\t" "return NIGHT_DECLINED\n\n", __func__);
	
    return NIGHT_DECLINED;
}

int
night_get_full_name(night_pool_t *pool, night_str_t *prefix, night_str_t *name)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function:\t" "%s\n\n", __func__);
	
    size_t      len;
    char     	*p, *n;
    int			rc;

	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"检查 name 是否已经是完整路径\n" "night_test_full_name\n\n");
	
    rc = night_test_full_name(name);
    
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"检查 name 是否已经是完整路径 完成\n" "night_test_full_name completed\n");
	dprintf(trace_file_fd,"rc=%d\n\n", rc);

    if (rc == NIGHT_OK) 
    {	
    	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd,"已经是完整路径\n" "function %s:\t" "return rc(%d)\n\n", __func__, rc);
    	
        return rc;
    }

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "不是完整路径\n\n");
	
    len = prefix->len;

    n = night_pnalloc(pool, len + name->len + 1);
    if (n == NULL) 
    {
        return NIGHT_ERROR;
    }

    p = night_cpymem(n, prefix->data, len);
    night_cpystrn(p, name->data, name->len + 1);

    name->len += len;
    name->data = n;

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "name=%s\n", name->data);
	dprintf(trace_file_fd, "function %s:\t" "return NIGHT_OK\n\n", __func__);
	
    return NIGHT_OK;
}



