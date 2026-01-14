#include "night_core.h"

static void
night_destroy_cycle_pools(night_conf_t *conf);

night_cycle_t *
night_init_cycle(night_cycle_t *old_cycle)
{
	night_time_t			*tp;
	night_pool_t			*pool;
	night_cycle_t			*cycle;
	night_cycle_t			**old;
	night_list_part_t		*part;
	// paths 数组 初始 size
	size_t					n;
	char					hostname[NIGHT_MAXHOSTNAMELEN];
	size_t					i;
	night_core_module_ctx_t	*module;
	void					*rv;
	char					**senv;
	night_conf_t			conf;
	
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_init_cycle\n\n");
	
	// 更新时区信息
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "更新时区信息\n" "night_timezone_update()\n\n");
	
    night_timezone_update();
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "更新时区信息 完成\n" "night_timezone_update() completed\n\n");
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "更新时间缓存\n" "night_time_update\n\n");
	
    tp = (night_time_t *) night_cached_time;
    tp->sec = 0;
		
    night_time_update();

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "更新时间缓存 完成\n" "night_time_update completed\n\n");
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "创建内存池\n" "night_create_pool\n\n");
	
    pool = night_create_pool(night_pagesize);
    if (pool == NULL) 
    {
        return NULL;
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "创建内存池 完成\n" "night_create_pool completed\n\n");
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "分配 night_cycle_t 结构体\n\n");

	cycle = night_pcalloc(pool, sizeof(night_cycle_t));
    if (cycle == NULL) 
    {
        night_destroy_pool(pool);
        return NULL;
    }
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "分配 night_cycle_t 结构体 完成\n\n");
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "初始化 cycle 基本字段：内存池、日志、指向旧 cycle\n\n");
	
    cycle->pool = pool;
    cycle->old_cycle = old_cycle;
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "从旧的 cycle（old_cycle）中复制关键配置路径和文件名字符串到新创建的 cycle 中\n\n");
	
	cycle->conf_prefix.len = old_cycle->conf_prefix.len;
    cycle->conf_prefix.data = night_pstrdup(pool, &old_cycle->conf_prefix);
    if (cycle->conf_prefix.data == NULL) 
    {
        night_destroy_pool(pool);
        return NULL;
    }

    cycle->prefix.len = old_cycle->prefix.len;
    cycle->prefix.data = night_pstrdup(pool, &old_cycle->prefix);
    if (cycle->prefix.data == NULL) 
    {
        night_destroy_pool(pool);
        return NULL;
    }

    cycle->conf_file.len = old_cycle->conf_file.len;
    cycle->conf_file.data = night_pnalloc(pool, old_cycle->conf_file.len + 1);
    if (cycle->conf_file.data == NULL) 
    {
        night_destroy_pool(pool);
        return NULL;
    }
    night_cpystrn(cycle->conf_file.data, old_cycle->conf_file.data, old_cycle->conf_file.len + 1);

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "从旧的 cycle（old_cycle）中复制关键配置路径和文件名字符串到新创建的 cycle 中 完成\n\n");
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "初始化 paths 数组\n\n");
	
    n = old_cycle->paths.nelts ? old_cycle->paths.nelts : 10;

    if (night_array_init(&cycle->paths, pool, n, sizeof(night_path_t *))
        != NIGHT_OK)
    {
        night_destroy_pool(pool);
        return NULL;
    }

    night_memzero(cycle->paths.elts, n * sizeof(night_path_t *));

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "初始化 paths 数组 完成\n\n");
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "初始化 config_dump \n\n");

    if (night_array_init(&cycle->config_dump, pool, 1, sizeof(night_conf_dump_t))
        != NIGHT_OK)
    {
        night_destroy_pool(pool);
        return NULL;
    }

    night_rbtree_init(&cycle->config_dump_rbtree, &cycle->config_dump_sentinel, night_str_rbtree_insert_value);        
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "初始化 config_dump 完成\n\n");   
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "初始化 open_files 链表\n\n");

    if (old_cycle->open_files.part.nelts) 
    {
        n = old_cycle->open_files.part.nelts;
        for (part = old_cycle->open_files.part.next; part; part = part->next) 
        {
            n += part->nelts;
        }
    }
	else 
	{
        n = 20;
    }

    if (night_list_init(&cycle->open_files, pool, n, sizeof(night_open_file_t))
        != NIGHT_OK)
    {
        night_destroy_pool(pool);
        return NULL;
    }

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "初始化 open_files 链表 完成\n\n");
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "初始化 shared_memory 链表\n\n");
	
    if (old_cycle->shared_memory.part.nelts) 
    {
        n = old_cycle->shared_memory.part.nelts;
        for (part = old_cycle->shared_memory.part.next; part; part = part->next)
        {
            n += part->nelts;
        }

    } 
    else 
    {
        n = 1;
    }

    if (night_list_init(&cycle->shared_memory, pool, n, sizeof(night_shm_zone_t))
        != NIGHT_OK)
    {
        night_destroy_pool(pool);
        return NULL;
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "初始化 shared_memory 链表 完成\n\n");
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "初始化 listening 数组\n\n");

    n = old_cycle->listening.nelts ? old_cycle->listening.nelts : 10;

    if (night_array_init(&cycle->listening, pool, n, sizeof(night_listening_t))
        != NIGHT_OK)
    {
        night_destroy_pool(pool);
        return NULL;
    }

    night_memzero(cycle->listening.elts, n * sizeof(night_listening_t));

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "初始化 listening 数组 完成\n\n");
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "初始化 可重用连接队列\n\n");

    night_queue_init(&cycle->reusable_connections_queue);
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "初始化 可重用连接队列 完成\n\n");
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "分配 模块配置上下文数组\n\n");

    cycle->conf_ctx = night_pcalloc(pool, night_modules_n * sizeof(void *));
    if (cycle->conf_ctx == NULL) 
    {
        night_destroy_pool(pool);
        return NULL;
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "分配 模块配置上下文数组 完成\n\n");
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "获取并设置主机名\n\n");

    if (gethostname(hostname, NIGHT_MAXHOSTNAMELEN) == -1) 
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "gethostname() failed\n\n");
    	
        night_destroy_pool(pool);
        return NULL;
    }


    hostname[NIGHT_MAXHOSTNAMELEN - 1] = '\0';
    cycle->hostname.len = strlen(hostname);

    cycle->hostname.data = night_pnalloc(pool, cycle->hostname.len);
    if (cycle->hostname.data == NULL) 
    {
        night_destroy_pool(pool);
        return NULL;
    }

    night_strlow(cycle->hostname.data, (char *) hostname, cycle->hostname.len);

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "获取并设置主机名 完成\n");
	dprintf(trace_file_fd, "hostname=%s\n\n", cycle->hostname.data);
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "加载所有模块到 cycle->modules\n\n");
	 
    if (night_cycle_modules(cycle) != NIGHT_OK) 
    {
        night_destroy_pool(pool);
        return NULL;
    }

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "加载所有模块到 cycle->modules 完成\n\n");
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "遍历所有模块,调用 CORE 模块的 create_conf 回调函数\n\n");
	
    for (i = 0; cycle->modules[i]; i++) 
    {
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "i=%ld\n\n", i);
    	
        if (cycle->modules[i]->type != NIGHT_CORE_MODULE) 
        {
            continue;
        }

        module = cycle->modules[i]->ctx;

        if (module->create_conf) 
        {
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(trace_file_fd, "module name=%s\n", cycle->modules[i]->name);
    		dprintf(trace_file_fd, "module->create_conf(cycle)\n\n");
    		
            rv = module->create_conf(cycle);
            if (rv == NULL) 
            {
                night_destroy_pool(pool);
                return NULL;
            }
            cycle->conf_ctx[cycle->modules[i]->index] = rv;
        }
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "遍历所有模块,调用 CORE 模块的 create_conf 回调函数 完成\n\n");
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "保存 environ 为了在解析完后恢复，防止污染全局环境\n\n");
	
    senv = environ;

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "初始化 配置解析器 conf, 为解析配置文件做准备\n\n");
	
    night_memzero(&conf, sizeof(night_conf_t));

    conf.args = night_array_create(pool, 10, sizeof(night_str_t));
    if (conf.args == NULL) 
    {
        night_destroy_pool(pool);
        return NULL;
    }

    conf.temp_pool = night_create_pool(night_pagesize);
    if (conf.temp_pool == NULL) 
    {
        night_destroy_pool(pool);
        return NULL;
    }

    conf.ctx = cycle->conf_ctx;
    conf.cycle = cycle;
    conf.pool = pool;
    conf.module_type = NIGHT_CORE_MODULE;
    conf.cmd_type = NIGHT_MAIN_CONF;

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "解析配置文件\n\n");
	
    if (night_conf_parse(&conf, &cycle->conf_file) != NIGHT_CONF_OK) 
    {
        environ = senv;
        night_destroy_cycle_pools(&conf);
        return NULL;
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "解析配置文件 完成\n\n");
}	

static void
night_destroy_cycle_pools(night_conf_t *conf)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "%s\n\n", __func__);
	
    night_destroy_pool(conf->temp_pool);
    night_destroy_pool(conf->pool);
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function %s:\t" "return\n\n", __func__);
}
