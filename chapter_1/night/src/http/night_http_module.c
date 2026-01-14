#include "night_core.h"

static int night_http_max_module;

static char *
night_http_merge_servers(night_conf_t *cf, night_http_core_main_conf_t *cmcf, night_http_module_ctx_t *module_ctx, int ctx_index)
{

} 

static int
night_http_init_locations(night_conf_t *cf, night_http_core_srv_conf_t *cscf, night_http_core_loc_conf_t *pclcf)   
{

}

static int
night_http_init_static_location_trees(night_conf_t *cf, night_http_core_loc_conf_t *pclcf)
{

}

static int
night_http_init_phases(night_conf_t *cf, night_http_core_main_conf_t *cmcf)
{

}

static int
night_http_init_headers_in_hash(night_conf_t *cf, night_http_core_main_conf_t *cmcf)
{

}

static int
night_http_init_phase_handlers(night_conf_t *cf, night_http_core_main_conf_t *cmcf)
{

}

static int
night_http_optimize_servers(night_conf_t *cf, night_http_core_main_conf_t *cmcf, night_array_t *ports)
{

}
    
static char *
night_http_block(night_conf_t *cf, night_command_t *cmd, void *conf)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "%s\n\n", __func__);
	
	night_http_conf_ctx_t			*ctx;
	int								m;
	int								mi;
	night_http_module_ctx_t			*module_ctx;
	char							*rv;
	night_conf_t					pcf;
	int								s;
    night_http_core_loc_conf_t    	*clcf;
    night_http_core_srv_conf_t   	**cscfp;
    night_http_core_main_conf_t   	*cmcf;
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "重复性检查\n\n");
	
	if (*(night_http_conf_ctx_t **) conf) 
	{
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "function %s:\t" "return \"is duplicate\"\n\n", __func__);
    	
        return "is duplicate";
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "为 http conf context 分配内存\n\n");
	
    ctx = night_pcalloc(cf->pool, sizeof(night_http_conf_ctx_t));
    if (ctx == NULL) 
    {
        return NIGHT_CONF_ERROR;
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "将 地址存入全局配置上下文\n\n");

    *(night_http_conf_ctx_t **) conf = ctx;
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "统计HTTP类型模块数量并分配索引\n\n");
	
    night_http_max_module = night_count_modules(cf->cycle, NIGHT_HTTP_MODULE);

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "统计HTTP类型模块数量并分配索引 完成\n\n");
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "为 main,server,location 三级配置分配指针数组\n\n");
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "为 main 级配置分配指针数组\n\n");
	
    ctx->main_conf = night_pcalloc(cf->pool, sizeof(void *) * night_http_max_module);
    if (ctx->main_conf == NULL) 
    {
        return NIGHT_CONF_ERROR;
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "为 server 级配置分配指针数组\n\n");
	
    ctx->srv_conf = night_pcalloc(cf->pool, sizeof(void *) * night_http_max_module);
    if (ctx->srv_conf == NULL) 
    {
        return NIGHT_CONF_ERROR;
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "为 location 级配置分配指针数组\n\n");
	
    ctx->loc_conf = night_pcalloc(cf->pool, sizeof(void *) * night_http_max_module);
    if (ctx->loc_conf == NULL) 
    {
        return NIGHT_CONF_ERROR;
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "为 HTTP 类型模块创建配置结构体\n\n");
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "为 HTTP 类型模块创建配置结构体\n\n");
	
    for (m = 0; cf->cycle->modules[m]; m++) 
    {
        if (cf->cycle->modules[m]->type != NIGHT_HTTP_MODULE) 
        {
            continue;
        }

        module_ctx = cf->cycle->modules[m]->ctx;
        mi = cf->cycle->modules[m]->ctx_index;

        if (module_ctx->create_main_conf) 
        {
            ctx->main_conf[mi] = module_ctx->create_main_conf(cf);
            if (ctx->main_conf[mi] == NULL) 
            {
                return NIGHT_CONF_ERROR;
            }
        }

        if (module_ctx->create_srv_conf) 
        {
            ctx->srv_conf[mi] = module_ctx->create_srv_conf(cf);
            if (ctx->srv_conf[mi] == NULL) 
            {
                return NIGHT_CONF_ERROR;
            }
        }

        if (module_ctx->create_loc_conf) 
        {
            ctx->loc_conf[mi] = module_ctx->create_loc_conf(cf);
            if (ctx->loc_conf[mi] == NULL) 
            {
                return NIGHT_CONF_ERROR;
            }
        }
    }
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "为 HTTP 类型模块创建配置结构体 完成\n\n");
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "保存原始配置上下文并设置新上下文\n\n");
	
	pcf = *cf;
    cf->ctx = ctx;
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "执行 HTTP 类型模块的 预配置 preconfiguration 函数\n\n");

    for (m = 0; cf->cycle->modules[m]; m++) 
    {
        if (cf->cycle->modules[m]->type != NIGHT_HTTP_MODULE) 
        {
            continue;
        }

        module_ctx = cf->cycle->modules[m]->ctx;

        if (module_ctx->preconfiguration) 
        {
            if (module_ctx->preconfiguration(cf) != NIGHT_OK) 
            {
                return NIGHT_CONF_ERROR;
            }
        }
    }
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "执行 HTTP 类型模块的 预配置 preconfiguration 函数 完成\n\n");
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "解析 HTTP 配置块内部内容\n\n");
	
    cf->module_type = NIGHT_HTTP_MODULE;
    cf->cmd_type = NIGHT_HTTP_MAIN_CONF;
    rv = night_conf_parse(cf, NULL);

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "解析 HTTP 配置块内部内容 完成\n\n");
	
	if (rv != NIGHT_CONF_OK) 
	{
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "解析 HTTP 配置块内部内容 失败\n" "goto failed\n\n");
	
        goto failed;
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "获取 server 列表\n\n");
     
    cmcf = ctx->main_conf[night_http_core_module.ctx_index];
    cscfp = cmcf->servers.elts;
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "遍历 HTTP 类型模块\n\n");
	
    for (m = 0; cf->cycle->modules[m]; m++) 
    {
        if (cf->cycle->modules[m]->type != NIGHT_HTTP_MODULE) 
        {
            continue;
        }

        module_ctx = cf->cycle->modules[m]->ctx;
        mi = cf->cycle->modules[m]->ctx_index;

        if (module_ctx->init_main_conf) 
        {
        	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "init %s 模块的 main 级配置\n\n", cf->cycle->modules[m]->name);
        	
            rv = module_ctx->init_main_conf(cf, ctx->main_conf[mi]);
			
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "init %s 模块的 main 级配置 完成\n\n", cf->cycle->modules[m]->name);
        	
            if (rv != NIGHT_CONF_OK) 
            {
				dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        		dprintf(trace_file_fd, "init %s 模块的 main 级配置 失败\n" "goto failed\n\n", cf->cycle->modules[m]->name);
        	
                goto failed;
            }
        }
		
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "merge %s 模块的 main 级配置 与 server 级配置\n\n", cf->cycle->modules[m]->name);
        	
        rv = night_http_merge_servers(cf, cmcf, module_ctx, mi);
        
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "merge %s 模块的 main 级配置 与 server 级配置 完成\n\n", cf->cycle->modules[m]->name);
		
        if (rv != NIGHT_CONF_OK) 
        {
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(trace_file_fd, "merge %s 模块的 main 级配置 与 server 级配置 失败\n" 
									"goto failed\n\n", cf->cycle->modules[m]->name);
		
            goto failed;
        }
    }


	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "遍历所有 server,创建其 location 树\n\n");
	
    for (s = 0; s < cmcf->servers.nelts; s++) 
    {
        clcf = cscfp[s]->ctx->loc_conf[night_http_core_module.ctx_index];

		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "整理 locations \n\n");
		
        if (night_http_init_locations(cf, cscfp[s], clcf) != NIGHT_OK) 
        {
            return NIGHT_CONF_ERROR;
        }
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "整理 locations 完成\n\n");

		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "利用整理好的 locations 构建二叉查找树\n\n");
		
        if (night_http_init_static_location_trees(cf, clcf) != NIGHT_OK) 
        {
            return NIGHT_CONF_ERROR;
        }
        
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "利用整理好的 locations 构建二叉查找树 完成\n\n");
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "遍历所有 server,创建其 location 树 完成\n\n");

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "初始化 11 个 HTTP 处理阶段\n\n");

    if (night_http_init_phases(cf, cmcf) != NIGHT_OK) 
    {
        return NIGHT_CONF_ERROR;
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "初始化 11 个 HTTP 处理阶段 完成\n\n");
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "构建请求头哈希表\n\n");

    if (night_http_init_headers_in_hash(cf, cmcf) != NIGHT_OK) 
    {
        return NIGHT_CONF_ERROR;
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "构建请求头哈希表 完成\n\n");

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "遍历 HTTP 类型的模块,执行 postconfiguration\n\n");
	
    for (m = 0; cf->cycle->modules[m]; m++) 
    {
        if (cf->cycle->modules[m]->type != NIGHT_HTTP_MODULE) 
        {
            continue;
        }

        module_ctx = cf->cycle->modules[m]->ctx;

        if (module_ctx->postconfiguration) 
        {
        	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "执行 %s 模块的 postconfiguration\n\n", cf->cycle->modules[m]->name);
        	
            if (module_ctx->postconfiguration(cf) != NIGHT_OK) 
            {
                return NIGHT_CONF_ERROR;
            }
        }
    }
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "遍历 HTTP 类型的模块,执行 postconfiguration 完成\n\n");
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "变量系统初始化\n\n");

    if (night_http_variables_init_vars(cf) != NIGHT_OK) 
    {
        return NIGHT_CONF_ERROR;
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "变量系统初始化 完成\n\n");
     
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "恢复原始配置上下文\n\n");

    *cf = pcf;

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "组装请求处理引擎,将各模块 handler 按 phase 合并为连续数组\n\n");

    if (night_http_init_phase_handlers(cf, cmcf) != NIGHT_OK) 
    {
        return NIGHT_CONF_ERROR;
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "组装请求处理引擎,将各模块 handler 按 phase 合并为连续数组 完成\n\n");

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "优化监听与虚拟主机\n\n");
	
    if (night_http_optimize_servers(cf, cmcf, cmcf->ports) != NIGHT_OK) 
    {
        return NIGHT_CONF_ERROR;
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "优化监听与虚拟主机 完成\n\n");

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function %s:\t" "return NIGHT_CONF_OK\n\n", __func__);
	
    return NIGHT_CONF_OK;

failed:
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "错误处理\n" "恢复原始配置上下文\n" 
							"function %s:\t" "return \n\n", __func__);
	
    *cf = pcf;

    return rv;
}


static night_command_t	night_http_commands[] = 
{
    { 
    	night_string("http"),
      	NIGHT_MAIN_CONF | NIGHT_CONF_BLOCK | NIGHT_CONF_NOARGS,
      	night_http_block,
      	0,
      	0,
      	NULL
	},

	night_null_command
};


static night_core_module_ctx_t	night_http_module_ctx = 
{
    night_string("http"),
    NULL,
    NULL
};


night_module_t night_http_module = 
{
    "night_http_module",					// name
	NIGHT_MODULE_UNSET_INDEX,				// index
    NIGHT_MODULE_UNSET_INDEX,				// ctx_index
    NIGHT_CORE_MODULE,						// type 
	&night_http_module_ctx,					// module context
    night_http_commands						// module directives
};
