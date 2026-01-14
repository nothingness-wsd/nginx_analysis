#include "night_core.h"

size_t 		night_modules_n;

night_module_t  *night_modules[] =
{
	&night_core_module,
	&night_events_module,
	&night_event_core_module,
	&night_epoll_module,
	&night_http_module,
	&night_http_core_module,
	NULL
};

static int
night_module_ctx_index(night_cycle_t *cycle, uint64_t type, int index)
{
    int				i;
    night_module_t	*module;
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "%s\n\n", __func__);
	
again:

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "循环 遍历所有模块 检查索引 %d 是否已经被使用\n\n", index);
	
    for (i = 0; cycle->modules[i]; i++) 
    {
        module = cycle->modules[i];
		
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "module name=%s\n\n", module->name);
	
        if (module->type != type) 
        {	
        	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "该模块类型不匹配，跳过\n\n");
        	
            continue;
        }

        if (module->ctx_index == index) 
        {
        	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "索引 %d 已使用，index++;\ngoto again;\n\n", index);
        	
            index++;
            goto again;
        }
    }

    /* check previous cycle */
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "遍历 old_cycle，避免 索引 冲突\n\n");
	
    if (cycle->old_cycle && cycle->old_cycle->modules) {

        for (i = 0; cycle->old_cycle->modules[i]; i++) {
            module = cycle->old_cycle->modules[i];

            if (module->type != type) {
                continue;
            }

            if (module->ctx_index == index) {
            	
				dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        		dprintf(trace_file_fd, "索引 %d 已使用，index++;\ngoto again;\n\n", index);
        	
                index++;
                goto again;
            }
        }
    }

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function %s:\t" "return %d\n\n", __func__,index);

    return index;
}

// 模块预初始化函数
int
night_preinit_modules(void)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_preinit_modules\n\n");
	
    size_t  	i;

	// 开始一个 for 循环，遍历 night_modules 数组；
	// night_modules 是一个以 NULL 结尾的模块指针数组
	// 每次循环处理一个模块
    for (i = 0; night_modules[i]; i++) 
    {
    	// 设置其 index 字段
        night_modules[i]->index = i;
    }

	// 表示静态模块的总数
    night_modules_n = i;

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function night_preinit_modules:\t" "return NIGHT_OK\n\n");
	
    return NIGHT_OK;
}

int
night_cycle_modules(night_cycle_t *cycle)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "%s\n\n", __func__);

    cycle->modules = night_pcalloc(cycle->pool, (night_modules_n + 1) * sizeof(night_module_t *));
    if (cycle->modules == NULL) 
    {
        return NIGHT_ERROR;
    }

    memcpy(cycle->modules, night_modules, night_modules_n * sizeof(night_module_t *));

    cycle->modules_n = night_modules_n;

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function %s:\t" "return NIGHT_OK\n\n", __func__);
	
    return NIGHT_OK;
}

int
night_count_modules(night_cycle_t *cycle, uint64_t type)
{
    int     		i, next, max;
    night_module_t  *module;

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "%s\n\n", __func__);
	
    next = 0;
    max = 0;
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "遍历所有模块\n\n");
	
    for (i = 0; cycle->modules[i]; i++) 
    {
        module = cycle->modules[i];
		
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "module name=%s\n\n", module->name);
	
        if (module->type != type) 
        {
        	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "跳过类型不符合的模块\n\n");
        	
            continue;
        }

        if (module->ctx_index != NIGHT_MODULE_UNSET_INDEX) 
        {
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "该模块索引已存在\n\n");

            if (module->ctx_index > max) 
            {
                max = module->ctx_index;
            }

            if (module->ctx_index == next) 
            {
                next++;
            }

            continue;
        }

        /* search for some free index */
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "按序分配可用索引，从 %d 开始查找可用索引\n\n", next);
		
        module->ctx_index = night_module_ctx_index(cycle, type, next);
        
        dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(trace_file_fd, "分配索引完成，ctx_index=%d\n\n", module->ctx_index);

        if (module->ctx_index > max) 
        {
            max = module->ctx_index;
        }

        next = module->ctx_index + 1;
    }
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "max=%d\n\n", max);

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "兼容  old_cycle\n\n");
    
    if (cycle->old_cycle && cycle->old_cycle->modules) 
    {
        for (i = 0; cycle->old_cycle->modules[i]; i++) 
        {
            module = cycle->old_cycle->modules[i];

            if (module->type != type) 
            {
                continue;
            }

            if (module->ctx_index > max) 
            {
                max = module->ctx_index;
                
				dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    			dprintf(trace_file_fd, "更新  max 为 old_cycle 的最大索引 %d\n\n", max);
            }
        }
    }
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "max=%d\n\n", max);

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function %s:\t" "return %d\n\n", __func__, max + 1);
	
    return max + 1;
}


