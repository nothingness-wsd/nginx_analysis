#include "night_core.h"
#include "night_http_module.h"
#include "night_module.h"
#include "night_core_module_ctx.h"
#include "night_conf.h"
#include "night_pool.h"
#include "night_cycle.h"
#include "night_modules.h"
#include "night_http_module_ctx.h"
#include "night_http_core_module.h"
#include "night_file_name.h"
#include "night_http_port.h"
#include "night_http_listen_opt.h"
#include "night_listening.h"
#include "night_string.h"
#include "night_http_connection.h"
#include "night_hash.h"


night_command_t night_http_commands[] = 
{
	{
		night_string("http"),
		NIGHT_MAIN_CONF,
		0,
		night_http_block
	},
	night_null_command
};

night_core_module_ctx_t night_http_module_ctx = 
{
	NULL,
	NULL
};

night_module_t night_http_module = 
{
	"night_http_module",
	NIGHT_MODULE_UNSET_INDEX,
	NIGHT_MODULE_UNSET_INDEX,
	NIGHT_CORE_MODULE,
	night_http_commands,
	&night_http_module_ctx,
	NULL,
	NULL,
	NULL
};

night_http_output_header_filter_pt  night_http_top_header_filter;
night_http_output_body_filter_pt    night_http_top_body_filter;
night_http_request_body_filter_pt   night_http_top_request_body_filter;

int night_http_modules_n;

int
night_http_block(night_conf_t *cf, night_command_t *cmd, void *conf)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_http_block\n\n");
	
	night_http_conf_t				*hc; 
	int 							i;
	night_module_t					*module;
	night_http_module_ctx_t			*module_ctx;
	int 							mi;
	night_conf_t 					pcf;
	int 							rc;
	night_http_core_main_conf_t   	*cmcf;
	
	
	hc = *(night_http_conf_t**) conf;
	if (hc)
	{
		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	    dprintf(error_log_fd,"http block configuration is duplicate\n\n");
	    
        return NIGHT_ERROR;	
	}
	
	hc = night_pmalloc(night_cycle->pool, sizeof(night_http_conf_t));
	if (hc == NULL)
	{
		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(error_log_fd, "night_pmalloc failed to allocate memory for night_http_conf_t while night_http_block\n\n");
		
		return NIGHT_ERROR;
	}
	
	*(night_http_conf_t**) conf = hc;
	
	night_http_modules_n = night_count_modules(NIGHT_HTTP_MODULE);
	
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"night_http_modules_n=%d\n\n", night_http_modules_n);
	
	hc->main_conf = night_pmalloc(night_cycle->pool, night_http_modules_n * sizeof(void*));
	if (hc->main_conf == NULL)
	{
		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(error_log_fd, "night_pmalloc failed to allocate memory for night_http_conf_t's main_conf while night_http_block\n\n");
		
		return NIGHT_ERROR;
	}
	
	hc->srv_conf = night_pmalloc(night_cycle->pool, night_http_modules_n * sizeof(void*));
	if (hc->srv_conf == NULL)
	{
		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(error_log_fd, "night_pmalloc failed to allocate memory for night_http_conf_t's srv_conf while night_http_block\n\n");
		
		return NIGHT_ERROR;
	}
	
	hc->loc_conf = night_pmalloc(night_cycle->pool, night_http_modules_n * sizeof(void*));
	if (hc->loc_conf == NULL)
	{
		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(error_log_fd, "night_pmalloc failed to allocate memory for night_http_conf_t's loc_conf while night_http_block\n\n");
		
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
        
        if(module_ctx->create_main_conf)
        {
        	 hc->main_conf[mi] = module_ctx->create_main_conf(cf);
        	 if (hc->main_conf[mi] == NULL)
        	 {
        	 	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	 	dprintf(error_log_fd, "\"%s\" create_main_conf failed while night_http_block\n\n",module->name);
        	 	
        	 	return NIGHT_ERROR;
        	 }
        }
        
        if (module_ctx->create_srv_conf)
        {
        	hc->srv_conf[mi] = module_ctx->create_srv_conf(cf);
        	if (hc->srv_conf[mi] == NULL)
        	{
        		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        		dprintf(error_log_fd, "\"%s\" create_srv_conf failed while night_http_block\n\n",module->name);
        	 	
        	 	return NIGHT_ERROR;
        	}
        }
        
        if (module_ctx->create_loc_conf)
        {
        	hc->loc_conf[mi] = module_ctx->create_loc_conf(cf);
        	if (hc->loc_conf[mi] == NULL)
        	{
        		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        		dprintf(error_log_fd, "\"%s\" create_loc_conf failed while night_http_block\n\n",module->name);
        	 	
        	 	return NIGHT_ERROR;
        	}
        }

	}
	
	pcf = *cf;
	
    cf->ctx = (void**) hc;
    
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"night_http_block:\nrc = night_conf_parse(cf, NULL);\n\n");
    
    rc = night_conf_parse(cf, NULL);
    if (rc == NIGHT_ERROR)
    {
    	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd,"http block configuration parse failed\n\n");
    	
    	return NIGHT_ERROR;
    }
	
    rc = night_http_init_location_trees(cf, hc);
    if (rc != NIGHT_OK)
    {
    	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd,"create_location_trees failed\n\n");
    	
    	return NIGHT_ERROR;
    }
    
    cmcf = hc->main_conf[night_http_core_module.ctx_index];
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "night_http_init_headers_in_hash\n\n");
    
	if (night_http_init_headers_in_hash(cf, cmcf) != NIGHT_OK) 
	{
		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(error_log_fd,"night_http_init_headers_in_hash failed\n\n");
		
        return NIGHT_ERROR;
    }
	
	// optimize the lists of ports, addresses and server names
    if (night_http_optimize_servers(cf, hc) != NIGHT_OK) 
    {
		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd,"night_http_optimize_servers failed\n\n");
        
        return NIGHT_ERROR;
    }
    dprintf(error_log_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(error_log_fd, "night_http_optimize_servers completed\n\n");
	
	//  HTTP 请求处理流程的 11 个标准阶段（phases）分配并初始化对应的 handler 链表
	if (night_http_init_phases(cf, cmcf) != NIGHT_OK) 
	{
        return NIGHT_ERROR;
    }
    
    // postconfiguration
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "NIGHT_HTTP_MODULE postconfiguration\n\n");
    
	for (i = 0; cf->cycle->modules[i]; i++) 
	{
		if (cf->cycle->modules[i]->type != NIGHT_HTTP_MODULE) 
		{
            continue;
        }
        
		module = cf->cycle->modules[i];
        module_ctx = module->ctx;
        mi = module->ctx_index;

        if (module_ctx->postconfiguration) 
        {
        	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "module->name=%s\n\n", module->name);
        	
            if (module_ctx->postconfiguration(cf) != NIGHT_OK) 
            {
                return NIGHT_ERROR;
            }
        }
    }
    
	// 初始化  HTTP 请求处理的“阶段处理器”（phase handlers）数组
	if (night_http_init_phase_handlers(cf, cmcf) != NIGHT_OK) 
	{
		dprintf(error_log_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd, "night_http_init_phase_handlers failed\n\n");
    	
        return NIGHT_ERROR;
    }
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "night_http_init_phase_handlers completed\n\n");
    
    *cf = pcf;
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "http_block parse completed\n\n");
	
	return NIGHT_OK;
}


int
night_http_init_phases(night_conf_t *cf, night_http_core_main_conf_t *cmcf)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_http_init_phases\n\n");
	
	// 读取完客户端请求头后的第一个阶段，常用于早期处理
    if (night_array_init(&cmcf->phases[NIGHT_HTTP_POST_READ_PHASE].handlers, night_cycle->pool, 1, sizeof(night_http_handler_pt)) != NIGHT_OK)
    {
        return NIGHT_ERROR;
    }
	
	// 在 server 块中执行 rewrite 规则（如 set, rewrite 指令），发生在 location 匹配之前
    if (night_array_init(&cmcf->phases[NIGHT_HTTP_SERVER_REWRITE_PHASE].handlers, night_cycle->pool, 1, sizeof(night_http_handler_pt)) != NIGHT_OK)
    {
        return NIGHT_ERROR;
    }

	// 在 location 匹配之后，再次执行 rewrite 规则。
    if (night_array_init(&cmcf->phases[NIGHT_HTTP_REWRITE_PHASE].handlers, night_cycle->pool, 1, sizeof(night_http_handler_pt)) != NIGHT_OK)
    {
        return NIGHT_ERROR;
    }
    
	// 访问控制前的准备阶段，常用于限速、连接数限制等。
    if (night_array_init(&cmcf->phases[NIGHT_HTTP_PREACCESS_PHASE].handlers, night_cycle->pool, 2, sizeof(night_http_handler_pt)) != NIGHT_OK)
    {
        return NIGHT_ERROR;
    }

	// 执行访问控制（如 IP 白名单/黑名单）
    if (night_array_init(&cmcf->phases[NIGHT_HTTP_ACCESS_PHASE].handlers, night_cycle->pool, 2, sizeof(night_http_handler_pt)) != NIGHT_OK)
    {
        return NIGHT_ERROR;
    }

	// 在生成响应内容前的最后处理阶段。
    if (night_array_init(&cmcf->phases[NIGHT_HTTP_PRECONTENT_PHASE].handlers, night_cycle->pool, 2, sizeof(night_http_handler_pt)) != NIGHT_OK)
    {
        return NIGHT_ERROR;
    }

	// 生成响应内容的核心阶段。静态文件服务、反向代理、FastCGI、uWSGI 等都在此阶段。
    if (night_array_init(&cmcf->phases[NIGHT_HTTP_CONTENT_PHASE].handlers, night_cycle->pool, 4, sizeof(night_http_handler_pt)) != NIGHT_OK)
    {
        return NIGHT_ERROR;
    }

	// 请求处理完成后记录日志
    if (night_array_init(&cmcf->phases[NIGHT_HTTP_LOG_PHASE].handlers, night_cycle->pool, 1, sizeof(night_http_handler_pt)) != NIGHT_OK)
    {
        return NIGHT_ERROR;
    }

    return NIGHT_OK;
}

// 它的作用是将配置阶段（如 rewrite、access 等）中注册的所有 handler（处理函数）按照执行顺序组织成一个线性数组（phase_engine.handlers），
// 并为每个 handler 设置对应的 checker 函数（用于控制执行流程，如是否继续、跳转、返回等）和 next 指针（用于跳转到下一阶段）
int
night_http_init_phase_handlers(night_conf_t *cf, night_http_core_main_conf_t *cmcf)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function\t" "night_http_init_phase_handlers\n\n");
	
	// 记录 NIGHT_HTTP_FIND_CONFIG_PHASE 在 handlers 数组中的索引
	int                  			find_config_index;
	// use_rewrite / use_access：标志位，表示 rewrite 或 access 阶段是否有注册 handler
	int								use_rewrite, use_access;
	int								n;
	int								i;
	night_http_phase_handler_t   	*ph;
	night_http_handler_pt			*h;
	night_http_phase_handler_pt   	checker;
	int								j;
	
	// 初始化特殊阶段索引为无效值, -1表示“未设置”
	cmcf->phase_engine.server_rewrite_index =  -1;
    cmcf->phase_engine.location_rewrite_index =  -1;
	//先设为 0，后面在处理 NIGHT_HTTP_FIND_CONFIG_PHASE 时会更新为真实索引
    find_config_index = 0;
    
    // 判断 rewrite 和 access 阶段是否被使用
    // 如果 rewrite 或 access 阶段有注册 handler，则对应标志为 1，否则为 0。
    use_rewrite = cmcf->phases[NIGHT_HTTP_REWRITE_PHASE].handlers.nelts ? 1 : 0;
    use_access = cmcf->phases[NIGHT_HTTP_ACCESS_PHASE].handlers.nelts ? 1 : 0;
    
	// 精确计算 HTTP 请求处理引擎（phase engine）所需的总 handler 数量 n,为后续分配内存做准备
	// NIGHT_HTTP_FIND_CONFIG_PHASE 是一个特殊的内部阶段，
	// 它没有用户注册的 handler，但需要一个 phase_handler 条目来存放其 checker 函数（ngx_http_core_find_config_phase）
	// 这个阶段的作用是在 rewrite 后重新查找匹配的 location（例如内部重定向）
	n = 1                  // find config phase
        + use_rewrite      // post rewrite phase
        + use_access;      // post access phase
        
    
	for (i = 0; i < NIGHT_HTTP_LOG_PHASE; i++) 
	{
        n += cmcf->phases[i].handlers.nelts;
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "n=%d\n\n", n);
    
    // 分配内存给 phase_engine.handlers
    ph = night_pmalloc(night_cycle->pool, n * sizeof(night_http_phase_handler_t) + sizeof(void *));
    if (ph == NULL) 
    {
        return NIGHT_ERROR;
    }
    
    // 将分配的数组保存到 cmcf->phase_engine.handlers。
	// 重置 n = 0，作为写入索引
	cmcf->phase_engine.handlers = ph;
    n = 0;
    
    // 遍历所有 phase（除 LOG_PHASE）
    for (i = 0; i < NIGHT_HTTP_LOG_PHASE; i++) 
    {
    	// h 指向当前 phase 的 handler 数组首地址
    	h = cmcf->phases[i].handlers.elts;
    	
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "h = cmcf->phases[i].handlers.elts;\n\n");
    	
    	switch (i) 
    	{
			case NIGHT_HTTP_SERVER_REWRITE_PHASE:
				// 如果尚未设置 server_rewrite_index，则记录当前 n（即该阶段第一个 handler 的位置）
				// 使用 night_http_core_rewrite_phase 作为 checker（支持 NGX_DECLINED, NGX_AGAIN 等返回值处理）。
            	if (cmcf->phase_engine.server_rewrite_index == -1) 
            	{
                	cmcf->phase_engine.server_rewrite_index = n;
            	}
            	checker = night_http_core_rewrite_phase;

            	break;
            	
			case NIGHT_HTTP_FIND_CONFIG_PHASE:
				// 这是一个无用户 handler的阶段，仅有一个 checker
				// 记录位置
            	find_config_index = n;

            	ph->checker = night_http_core_find_config_phase;
            	n++;
            	ph++;

            	continue;
            	
            case NIGHT_HTTP_REWRITE_PHASE:
            	
				if (cmcf->phase_engine.location_rewrite_index == -1) 
				{
                	cmcf->phase_engine.location_rewrite_index = n;
            	}
            	checker = night_http_core_rewrite_phase;
            
            	break;
            	
            case NIGHT_HTTP_POST_REWRITE_PHASE:
				if (use_rewrite) 
				{
                	ph->checker = night_http_core_post_rewrite_phase;
                	// 如果需要重试，跳转到 find_config 阶段
                	ph->next = find_config_index;
                	n++;
                	ph++;
            	}

            	continue;

            case NIGHT_HTTP_ACCESS_PHASE:
				checker = night_http_core_access_phase;
				// 这里 n++ 是为了后续 post_access 的 next 指向正确位置
            	n++;
            	break;
            	
            case NIGHT_HTTP_POST_ACCESS_PHASE:
				if (use_access) 
				{
                	ph->checker = night_http_core_post_access_phase;
                	// post_access 之后继续执行下一个阶段（即 n 当前值，已在 access 阶段处理时 n++ 过）
                	ph->next = n;
                	ph++;
            	}

            	continue;
            
            case NIGHT_HTTP_CONTENT_PHASE:
				checker = night_http_core_content_phase;
            	break;
            	
            default:	
            	checker = night_http_core_generic_phase;		
    	}
    	
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "n += cmcf->phases[i].handlers.nelts;\n");
    	dprintf(trace_file_fd, "%d += %ld\n", n, cmcf->phases[i].handlers.nelts);
    	dprintf(trace_file_fd,"n=%d\n\n", n);
    	
    	// 处理当前 phase 的所有用户 handler
    	n += cmcf->phases[i].handlers.nelts;
    	// 倒序遍历 handler 数组（j 从 nelts-1 到 0）
		for (j = cmcf->phases[i].handlers.nelts - 1; j >= 0; j--) 
		{
            ph->checker = checker;
            ph->handler = h[j];
            ph->next = n;
            ph++;
        }
	}
    
	return NIGHT_OK;
}

int 
night_http_init_location_trees(night_conf_t *cf, night_http_conf_t *hc)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function:\t" "night_http_init_location_trees\n\n");
	
	night_http_core_main_conf_t	*mc;
    night_http_core_srv_conf_t 	**servers;
    night_http_core_loc_conf_t 	*lc;
    int 						s;
    int 						ctx_index;
    int	 						rc;
    size_t						n;
    
    ctx_index = night_http_core_module.ctx_index;
    mc = hc->main_conf[ctx_index];
    servers = mc->servers.elts;
    
    /* create location trees */
    n = mc->servers.nelts;
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "servers.nelts=%ld\n\n", n);
    
    for (s = 0; s < n ; s++ )
    {   
        lc = servers[s]->ctx->loc_conf[ctx_index];
        
        rc = night_http_init_locations(cf, lc);
        if (rc != NIGHT_OK)
        {
        	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(error_log_fd,"night_http_init_locations failed\n\n");
        	
        	return NIGHT_ERROR;
        }
        
        rc = night_http_init_static_location_trees(cf, lc);
		if (rc != NIGHT_OK)
		{
		    dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(error_log_fd,"night_http_init_static_location_trees failed\n\n");
        	
        	return NIGHT_ERROR;
		} 
    }
    
    return NIGHT_OK;
}

int
night_http_init_locations(night_conf_t *cf, night_http_core_loc_conf_t *plc)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function:\t" "night_http_init_locations\n\n");
	
	night_http_core_loc_conf_t 	*lc;
	night_queue_t 				*locations;
	night_queue_t 				*q;
	night_http_location_queue_t *lq;
	int 						rc;
	
	locations = plc->locations;
	
	if (locations == NULL)
	{
		return NIGHT_OK;
	}
	
	if (night_queue_empty(locations)) 
	{
        return NIGHT_OK;
    }
    
    night_queue_sort(locations, night_http_cmp_locations);
    
    for (q = night_queue_head(locations); q != night_queue_sentinel(locations); q = night_queue_next(q))
    {
    	lq = (night_http_location_queue_t*) q;
    	
    	lc = lq->exact ? lq->exact : lq->inclusive;
    	
    	rc = night_http_init_locations(cf, lc);
    	if(rc != NIGHT_OK)
    	{
    		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    		dprintf(error_log_fd,"night_http_init_locations failed\n\n");
    		
    		return NIGHT_ERROR;
    	}

    }
	
	return NIGHT_OK;
}

int
night_http_cmp_locations(const night_queue_t *one, const night_queue_t *two)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_http_cmp_locations\n\n");
    
    night_http_location_queue_t 	*lq1, 	*lq2;
    night_http_core_loc_conf_t 		*first, *second;
    int 							rc;
    
    lq1 = (night_http_location_queue_t*) one;
    lq2 = (night_http_location_queue_t*) two;
    
    first = lq1->exact ? lq1->exact : lq1->inclusive;
    second = lq2->exact ? lq2->exact : lq2->inclusive;
    
    rc = night_filename_cmp(first->name.data, second->name.data, night_min(first->name.len, second->name.len) + 1);

    if(rc == 0 && !first->exact_match && second->exact_match) {
        // an exact match must be before the same inclusive one 
        return 1;
    }

    return rc; 
}

int
night_http_init_static_location_trees(night_conf_t *cf, night_http_core_loc_conf_t *plc)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function:\t" "night_http_init_static_location_trees\n\n");
	
	night_queue_t 				*locations;
	night_queue_t 				*q;
	night_http_location_queue_t *lq;
	night_http_core_loc_conf_t 	*lc;
	int 						rc;
	
	locations = plc->locations;
    if (locations == NULL) 
    {
        return NIGHT_OK;
    }
    
    if (night_queue_empty(locations)) 
    {
        return NIGHT_OK;
    }
    
    for (q = night_queue_head(locations); q != night_queue_sentinel(locations); q = night_queue_next(q))
    {
        lq = (night_http_location_queue_t*) q;
    	lc = lq->exact ? lq->exact : lq->inclusive;
    	
    	rc = night_http_init_static_location_trees(cf, lc);
    	if (rc != NIGHT_OK)
    	{
    		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    		dprintf(error_log_fd,"night_http_init_static_location_trees failed\n\n");
    		
    		return NIGHT_ERROR;
    	}
    }
	
	rc = night_http_join_exact_locations(cf, locations);
	if (rc != NIGHT_OK)
	{
		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd,"night_http_join_exact_locations failed\n\n");
		
		return NIGHT_ERROR;
	}
	
	night_http_create_locations_list(locations, night_queue_head(locations));
	
	plc->static_locations_tree = night_http_create_locations_tree(cf, locations, 0);
    if (plc->static_locations_tree == NULL) 
    {
        return NIGHT_ERROR;
    }
	
	return NIGHT_OK;
}

int
night_http_join_exact_locations(night_conf_t *cf, night_queue_t *locations)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function:\t" "night_http_join_exact_locations\n\n");
	
	night_queue_t 				*q;
	night_queue_t 				*x;
	night_http_location_queue_t *lq;
	night_http_location_queue_t *lx;
	
	q = night_queue_head(locations);
	
	while (q != night_queue_last(locations)) 
	{
		x = night_queue_next(q);
		
		lq = (night_http_location_queue_t*)q;
        lx = (night_http_location_queue_t*)x;
        
        if ((lq->name->len == lx->name->len) &&
            	(night_filename_cmp(lq->name->data , lx->name->data , lx->name->len ) == 0))
        {
        	if ((lq->exact && lx->exact) || (lq->inclusive && lx->inclusive))
        	{
        		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
                dprintf(error_log_fd,"duplicate location %s in %s at line %d\n\n", 
                		lq->name->data,lq->file_name, lq->line);
                
                return NIGHT_ERROR;
            }
            
            lq->inclusive = lx->inclusive;
            night_queue_remove(x);
            
            continue;
        }
        q = night_queue_next(q);
	}
	
	return NIGHT_OK;
}

void
night_http_create_locations_list(night_queue_t *locations, night_queue_t *q)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function:\t" "night_http_create_locations_list\n\n");
	
	night_http_location_queue_t *lq;
	char 						*name;
    size_t 						len;
	night_queue_t 				*x;
	night_http_location_queue_t *lx;
	night_queue_t 				tail;
	
	if (q == night_queue_last(locations)) 
	{
        return;
    }
    
    lq = (night_http_location_queue_t*) q;
    
	if (lq->inclusive == NULL) 
	{
        night_http_create_locations_list(locations, night_queue_next(q));
        
        return;
    }
    
    len = lq->name->len;
    name = lq->name->data;
    
	for (x = night_queue_next(q); x != night_queue_sentinel(locations); x = night_queue_next(x))
    {
        lx = (night_http_location_queue_t*) x;

        if (len > lx->name->len
            || night_filename_cmp(name, lx->name->data, len) != 0)
        {
            break;
        }
    }
    
    q = night_queue_next(q);
    
    if (q == x) 
    {
        night_http_create_locations_list(locations, x);
        return;
    }
    
    night_queue_split(locations, q, &tail);
    night_queue_add(&lq->list, &tail);
    
    if (x == night_queue_sentinel(locations)) 
    {
        night_http_create_locations_list(&lq->list, night_queue_head(&lq->list));
        return;
    }
    
    night_queue_split(&lq->list, x, &tail);
    night_queue_add(locations, &tail);
    
    night_http_create_locations_list(&lq->list, night_queue_head(&lq->list));
    night_http_create_locations_list(locations, x);
}

night_http_location_tree_node_t*
night_http_create_locations_tree(night_conf_t *cf, night_queue_t *locations, size_t prefix)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function:\t" "night_http_create_locations_tree\n\n");
	
	night_http_location_tree_node_t		*node;
	night_queue_t						*q;
	night_http_location_queue_t 		*lq;
	size_t 								len;
	night_queue_t 						tail;
	
	q = night_queue_middle(locations);
	lq = (night_http_location_queue_t*) q;
	
	len = lq->name->len - prefix;
	
	node = night_pmalloc(night_cycle->pool, offsetof(night_http_location_tree_node_t, name) + len + 1);
	if (node == NULL)
	{
		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd,"night_pmalloc failed while night_http_create_locations_tree\n\n");
		
		return NULL;
	}
	
	node->left = NULL;
    node->right = NULL;
    node->tree = NULL;
    node->exact = lq->exact;
    node->inclusive = lq->inclusive;
    node->len = len;
    memcpy(node->name, &lq->name->data[prefix], len);
    
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"node->name=%s\n\n" , node->name);
	
	night_queue_split(locations, q, &tail);
    
    if (night_queue_empty(locations)) 
    {
        goto inclusive;
    }
    
	node->left = night_http_create_locations_tree(cf, locations, prefix);
    if (node->left == NULL) 
    {
        return NULL;
    }
	
	night_queue_remove(q);
	if (night_queue_empty(&tail)) 
	{
        goto inclusive;
    }
    
    node->right = night_http_create_locations_tree(cf, &tail, prefix);
    if (node->right == NULL) 
    {
        return NULL;
    }

inclusive:

    if (night_queue_empty(&lq->list)) 
    {
        return node;
    }

    node->tree = night_http_create_locations_tree(cf, &lq->list, prefix + len);
    if (node->tree == NULL) 
    {
        return NULL;
    }
    	
	return node;
}

int 
night_http_optimize_servers(night_conf_t *cf, night_http_conf_t *hc)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function:\t" "night_http_optimize_servers\n\n");
	
	night_http_core_main_conf_t	*mc;
	night_array_t				*ports;
	night_http_port_t			*port;
	int 						p;
	
	mc = hc->main_conf[night_http_core_module.ctx_index];
	ports = &mc->ports;
	
	if (ports == NULL)  
	{
        return NIGHT_OK;
    }
    
    port = ports->elts;
	for (p = 0; p < ports->nelts; p++) 
	{
        if (night_http_init_listening(cf, &port[p]) != NIGHT_OK) 
        {
			dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(error_log_fd,"night_http_init_listening failed\n\n");
            
            return NIGHT_ERROR;
        }
    }
    
	return NIGHT_OK;
}

int
night_http_init_listening(night_conf_t *cf, night_http_port_t *port)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function:\t" "night_http_init_listening\n\n");
	
	night_listening_t	*ls;
	
	ls = night_http_add_listening(cf, port);
    if (ls == NULL) 
    {
		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(error_log_fd,"night_http_add_listening failed\n\n");
        
        return NIGHT_ERROR;
    }
	
	return NIGHT_OK;
}

night_listening_t*
night_http_add_listening(night_conf_t *cf, night_http_port_t *port)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function:\t" "night_http_add_listening\n\n");
	
	night_listening_t *ls;
	
	ls = night_create_listening(cf, port->listen_opt->sockaddr, port->listen_opt->socklen);
    if (ls == NULL) 
    {
		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(error_log_fd,"night_create_listening failed\n\n");
        
        return NULL;
    }
    
    ls->backlog = port->listen_opt->backlog;
    ls->server = port->server;
    
    ls->handler = night_http_init_connection;
    
	return ls;
}

night_listening_t*
night_create_listening(night_conf_t *cf, struct sockaddr *sockaddr, socklen_t socklen)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function:\t" "night_create_listening\n\n");
	
	night_listening_t 	*ls;
	char 				text[NIGHT_SOCKADDR_STRLEN + 1];
	struct sockaddr		*sa;
	int 				len;
	
	ls = night_array_push(&cf->cycle->listening);
    if (ls == NULL) 
    {
		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(error_log_fd,"night_array_push failed\n\n");
        
        return NULL;
    }
    
	night_memzero(ls, sizeof(night_listening_t));
    night_memzero(text, NIGHT_SOCKADDR_STRLEN + 1);
	
	sa = night_pmalloc(night_cycle->pool, socklen);
    if (sa == NULL) 
    {
		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(error_log_fd,"night_pmalloc failed\n\n");
        
        return NULL;
    }
    
	memcpy(sa, sockaddr, socklen);
    
    ls->sockaddr = sa;
    ls->socklen = socklen;
    
    ls->addr_text_max_len = NIGHT_INET_ADDRSTRLEN;
    
	len = night_sock_ntop(sa, socklen, text, NIGHT_SOCKADDR_STRLEN + 1, 1);
    ls->addr_text.len = len;
    
	ls->addr_text.data = night_pmalloc(night_cycle->pool, len + 1);
    if (ls->addr_text.data == NULL) 
    {
		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(error_log_fd,"night_pmalloc failed\n\n");
        
        return NULL;
    }
    
	memcpy(ls->addr_text.data, text, len);
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "addr_text=%s\n" , ls->addr_text.data);
    dprintf(trace_file_fd, "len=%d\n\n",  len);
    
	ls->fd = -1;
    ls->type = SOCK_STREAM;

    ls->backlog = NIGHT_LISTEN_BACKLOG;
    ls->rcvbuf = -1;
    ls->sndbuf = -1;
    
    ls->worker = 0;
    
	return ls;
}

int
night_http_init_headers_in_hash(night_conf_t *cf, night_http_core_main_conf_t *cmcf)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_http_init_headers_in_hash\n\n");
	
	night_array_t         		headers_in;
	int							rc;
	night_http_header_t  		*header;
	night_hash_key_t     		*hk;
	night_hash_init_t     		hash;
	
	night_hash_elt_t			**buckets;
	night_hash_elt_t			*elt;
	size_t						size;
	size_t						i;
	
	
	// 初始化动态数组 headers_in
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "night_array_init headers_in\n\n");
	
	rc = night_array_init(&headers_in, cf->pool, 32, sizeof(night_hash_key_t));
	if (rc != NIGHT_OK)
    {
    	dprintf(error_log_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd, "night_array_init headers_in failed\n\n");
    	
        return NIGHT_ERROR;
    }
    
    // 遍历全局数组 night_http_headers_in
	for (header = night_http_headers_in; header->name.len; header++) 
	{
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(trace_file_fd, "hk = night_array_push(&headers_in);\n\n");
        
        hk = night_array_push(&headers_in);
        if (hk == NULL) 
        {
            return NIGHT_ERROR;
        }

        hk->key = header->name;
        hk->key_hash = night_hash_key_lc(header->name.data, header->name.len);
        hk->value = header;
        
        dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(trace_file_fd, "header->name=%s\n\n", header->name.data);
    }
    
    // 指定要初始化的目标哈希表地址
	hash.hash = &cmcf->headers_in_hash;
	// 设置哈希函数
    hash.key = night_hash_key_lc;
    // 设置哈希表的最大桶数（bucket 数量）为 512
    hash.max_size = night_page_size;
    // 设置每个哈希桶的大小
    hash.bucket_size = 64;
    // 为哈希表设置一个名称
    hash.name = "headers_in_hash";
    // 指定用于分配哈希表内存的内存池
    hash.pool = night_cycle->pool;
    // 设置临时内存池为 NULL
    hash.temp_pool = NULL;
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "night_hash_init\n\n");
    
    rc = night_hash_init(&hash, headers_in.elts, headers_in.nelts);
	if (rc != NIGHT_OK)
	{
		dprintf(error_log_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(error_log_fd, "night_hash_init failed\n\n");
		
        return NIGHT_ERROR;
    }
    
	// 查看 结果
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd, "hash table:\n\n");
    
    buckets = cmcf->headers_in_hash.buckets;
    size = cmcf->headers_in_hash.size;
    for (i = 0; i < size; i++)
    {
		if (buckets[i] == NULL) 
        {
            continue;
        }
        
        elt = buckets[i];
        
        while(elt->value)
        {
			//dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
			write(trace_file_fd, elt->name, elt->len);
        	dprintf(trace_file_fd, "\n");
        	
        	elt = (night_hash_elt_t*) night_align_ptr(&elt->name[0] + elt->len, sizeof(void *));
        } 
    }
    dprintf(trace_file_fd, "\n");
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "return NIGHT_OK;\n\n");
    
	return NIGHT_OK;
}

