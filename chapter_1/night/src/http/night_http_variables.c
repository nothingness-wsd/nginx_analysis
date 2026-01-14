#include "night_core.h"


static int
night_http_variable_host(night_http_request_t *r, night_http_variable_value_t *v, uintptr_t data)
{

}

static night_http_variable_t  night_http_core_variables[] = 
{
	{ night_string("host"), NULL, night_http_variable_host, 0, 0, 0 },
	night_http_null_variable
};




int
night_http_variables_add_core_vars(night_conf_t *cf)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "%s\n\n", __func__);
	
    night_http_variable_t        *cv, *v;
    night_http_core_main_conf_t  *cmcf;

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "获取核心模块主配置\n\n");
	
    cmcf = night_http_conf_get_module_main_conf(cf, night_http_core_module);

	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "为 variables_keys 分配内存");
	
    cmcf->variables_keys = night_pcalloc(cf->temp_pool, sizeof(night_hash_keys_arrays_t));
    if (cmcf->variables_keys == NULL) 
    {
        return NIGHT_ERROR;
    }

    cmcf->variables_keys->pool = cf->pool;
    cmcf->variables_keys->temp_pool = cf->pool;

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "初始化 variables_keys\n\n");
	
    if (night_hash_keys_array_init(cmcf->variables_keys, NIGHT_HASH_SMALL)
        != NIGHT_OK)
    {
        return NIGHT_ERROR;
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "初始化 variables_keys 完成\n\n");

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "初始化 prefix_variables\n\n");
    if (night_array_init(&cmcf->prefix_variables, cf->pool, 8, sizeof(night_http_variable_t))
        != NIGHT_OK)
    {
        return NIGHT_ERROR;
    }
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "初始化 prefix_variables 完成\n\n");

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "遍历并注册所有核心内置变量\n\n");

    for (cv = night_http_core_variables; cv->name.len; cv++) 
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "night_http_add_variable 注册变量名");
    	
        v = night_http_add_variable(cf, &cv->name, cv->flags);
        if (v == NULL) 
        {
            return NIGHT_ERROR;
        }
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "ngx_http_add_variable 注册变量名 完成");
		
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "复制变量完整内容\n\n");
		
        *v = *cv;
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "遍历并注册所有核心内置变量 完成\n\n");

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function %s:\t" "return NGX_OK\n\n", __func__);
	
    return NGX_OK;
}

int
night_http_variables_init_vars(night_conf_t *cf)
{

}
