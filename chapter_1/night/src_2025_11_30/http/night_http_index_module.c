#include "night_core.h"
#include "night_http_index_module.h"
#include "night_module.h"
#include "night_http_module.h"
#include "night_http_module_ctx.h"
#include "night_pool.h"
#include "night_cycle.h"
#include "night_conf.h"
#include "night_http_core_module.h"
#include "night_open_file_cache.h"
#include "night_string.h"


night_command_t night_http_index_commands[] = 
{
    {
        night_string("index"),
        NIGHT_HTTP_LOC_CONF,
        NIGHT_HTTP_LOC_CONF_OFFSET,
        night_http_index
    },
	night_null_command
};

night_http_module_ctx_t night_http_index_module_ctx = 
{
	NULL,								// create main configuration
	NULL,								// create server configuration
	night_http_index_create_loc_conf,	// create location configuration
	night_http_index_init				//	postconfiguration
};

night_module_t night_http_index_module = 
{
	"night_http_index_module",
	NIGHT_MODULE_UNSET_INDEX,
	NIGHT_MODULE_UNSET_INDEX,
	NIGHT_HTTP_MODULE,
	night_http_index_commands,
	&night_http_index_module_ctx,
	NULL,
	NULL,
	NULL
};

void*
night_http_index_create_loc_conf(night_conf_t *cf)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function:\t" "night_http_index_create_loc_conf\n\n");

    night_http_index_loc_conf_t	*lc;
    int 						rc;

    lc = night_pmalloc(night_cycle->pool, sizeof(night_http_index_loc_conf_t));
    if (lc == NULL) 
    {
    	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd,"night_pmalloc failed while night_http_index_create_loc_conf\n\n");
        
        return NULL;
    }
	
	rc = night_array_init(&lc->indices, night_cycle->pool, 2, sizeof(night_http_index_t));
	if (rc != NIGHT_OK)
	{
		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(error_log_fd,"night_array_init failed while night_http_index_create_loc_conf\n\n");
		
		return NULL;
	}
	
    lc->max_index_len = 0;

    return lc;
}

int 
night_http_index(night_conf_t *cf, night_command_t *cmd, void *conf)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function:\t" "night_http_index\n\n");
	
	night_http_index_loc_conf_t	*lc;
	night_str_t					*value;
	int 						n;
	night_http_index_t 			*index;
	int 						i;
	
	lc = (night_http_index_loc_conf_t*) conf;
	value = cf->args.elts;
	n = cf->args.nelts;
	
	for(i = 1; i < n; i++) 
	{
		index = night_array_push(&lc->indices);
		if (index == NULL)
		{
			dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(error_log_fd,"night_array_push failed while night_http_index\n\n");
			
			return NIGHT_ERROR;
		}
		
		index->name.len = value[i].len;
		index->name.data = night_pmalloc(night_cycle->pool, (index->name.len + 1));
		
		strcat(index->name.data, value[i].data);
		
		dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(trace_file_fd,"index=%s\n\n",index->name.data);
        
        if (lc->max_index_len < (index->name.len + 1))
        {
        	lc->max_index_len = index->name.len + 1;
        }
	}
	
	return NIGHT_OK;
}

int
night_http_index_init(night_conf_t *cf)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_http_index_init\n\n");
	
    night_http_handler_pt        *h;
    night_http_core_main_conf_t  *cmcf;

    cmcf = ((night_http_conf_t *) cf->ctx)->main_conf[night_http_core_module.ctx_index];

    h = night_array_push(&cmcf->phases[NIGHT_HTTP_CONTENT_PHASE].handlers);
    if (h == NULL) 
    {
        return NIGHT_ERROR;
    }

    *h = night_http_index_handler;
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "*h = night_http_index_handler;\n");
	dprintf(trace_file_fd, "return NIGHT_OK;\n\n");
	
    return NIGHT_OK;
}

// 这是一个 HTTP 请求处理函数（handler），用于在请求 URI 以 / 结尾时尝试返回一个索引文件
int
night_http_index_handler(night_http_request_t *r)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_http_index_handler\n\n");
	
	night_http_core_loc_conf_t     		*clcf;
    night_http_index_loc_conf_t    		*ilcf;
    size_t								allocated;
    size_t								root;
    uint64_t							dir_tested;
    char*								name;
    night_str_t                     	path, uri;
    night_http_index_t             		*index;
    size_t								reserve, len;
    uint64_t							i;
    night_open_file_info_t          	of;
    int 								rc;
    
	/*
	只有 URI 以 / 结尾（如 /dir/）才尝试找 index 文件。
	否则：直接 DECLINED，交给其他 handler（如 static module 处理具体文件）。
	*/
	if (r->uri.data[r->uri.len - 1] != '/') 
	{
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "if (r->uri.data[r->uri.len - 1] != '/')\n" "return NIGHT_DECLINED;\n\n");
		
        return NIGHT_DECLINED;
    }
    
    // 只处理 GET、HEAD、POST 请求
	if (!(r->method & (NIGHT_HTTP_GET | NIGHT_HTTP_HEAD | NIGHT_HTTP_POST))) 
	{
    
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "if (!(r->method & (NIGHT_HTTP_GET | NIGHT_HTTP_HEAD | NIGHT_HTTP_POST))) \n");
    	dprintf(trace_file_fd, "return NGX_DECLINED;\n\n");
    
        return NIGHT_DECLINED;
    }
    
	// 获取配置
	ilcf = r->loc_conf[night_http_index_module.ctx_index];
    clcf = r->loc_conf[night_http_core_module.ctx_index];

	// 初始化变量
	/*
	allocated：已分配的缓冲区大小（用于复用 path 缓冲区）。
	root：ngx_http_map_uri_to_path 返回的 root 路径长度。
	dir_tested：标记是否已验证父目录存在且可访问。
	path.data = NULL：避免未初始化警告（实际后续会被赋值）。
	*/
    allocated = 0;
    root = 0;
    dir_tested = 0;
    name = NULL;
    /* suppress MSVC warning */
    path.data = NULL;

	/*
	 遍历所有 index 文件
	ilcf->indices 是一个数组，保存所有 index 文件名（如 ["index.html", "index.php"]）。
	依次尝试每个 index 文件，直到找到存在的文件。
	*/
    index = ilcf->indices.elts;
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "ilcf->indices.nelts=%ld\n\n", ilcf->indices.nelts);
    
    for (i = 0; i < ilcf->indices.nelts; i++) 
    {
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "index[%ld]\n\n", i);
    	
    	// lengths == NULL 表示该 index 名是静态字符串
		if (index[i].lengths == NULL) 
		{
        
        	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "if (index[i].lengths == NULL)\n");
        	dprintf(trace_file_fd, "index[i].name.data=%s\n\n", index[i].name.data);
        	
        	// 如果 index 以 / 开头（绝对路径）
			if (index[i].name.data[0] == '/') 
			{
				dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
				dprintf(trace_file_fd, "if (index[i].name.data[0] == '/')\n\n");
				
				// 直接内部重定向到该 URI
                //return night_http_internal_redirect(r, &index[i].name, &r->args);
            }
            
			// 计算所需缓冲区大小
			// len：index 文件名长度（含结尾 \0）
			// reserve：预分配空间（max_index_len 是所有 index 名的最大长度）。
            reserve = ilcf->max_index_len;
            len = index[i].name.len + 1;
            
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            dprintf(trace_file_fd, "reserve=%ld\n", reserve);
            dprintf(trace_file_fd, "len=%ld\n\n", len);
		}  
            
		/*
		分配或复用 path 缓冲区
		作用：将 URI 映射为文件系统路径。
		name 指向 path 中“文件名”开始的位置（即 /var/www/dir/ 的末尾）
		allocated 记录剩余可用空间（从 name 到缓冲区末尾）
		*/
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "reserve=%ld\nallocated=%ld\n\n", reserve, allocated);
		
        if (reserve > allocated) 
        {
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(trace_file_fd, "if (reserve > allocated)\n\n");
			
			name = night_http_map_uri_to_path(r, &path, &root, reserve);
            if (name == NULL) 
            {
                return NIGHT_HTTP_INTERNAL_SERVER_ERROR;
            }

            allocated = path.data + path.len - name;
            
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            dprintf(trace_file_fd, "allocated=%ld\n\n", allocated);
        }
        
		// 构造完整文件路径（静态名）
		// 静态 index 名，直接拷贝到 name 位置
        if (index[i].values == NULL) 
        {
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(trace_file_fd, "if (index[i].values == NULL)\n\n");

            memcpy(name, index[i].name.data, index[i].name.len);

            path.len = (name + index[i].name.len) - path.data;	
            
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            dprintf(trace_file_fd, "path.len=%ld\n", path.len);
            dprintf(trace_file_fd, "path.data=%s\n\n", path.data);
		}
		

		//初始化文件信息结构
		// 清零结构体
        night_memzero(&of, sizeof(night_open_file_info_t));
		// 指定 open file cache 中缓存项的有效时间（秒）
        of.valid = 60;
        
        // 指定一个缓存项必须被访问 至少多少次，才值得被缓存
        of.min_uses = 1;
        
        // 设置为“仅测试”模式
        // 只检查文件是否存在和可访问，不要真正打开文件描述符（fd）
        of.test_only = 1;
        
		// 检查文件是否存在（使用缓存）
		// 查询 open file cache，检查文件状态
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "night_open_cached_file\n\n");
		
        if (night_open_cached_file(clcf->open_file_cache, &path, &of, r->pool) != NIGHT_OK)
        {
        	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "if night_open_cached_file() != NIGHT_OK\n\n");
        	
			// 文件不存在，尝试下一个 index 文件
            if (of.err == ENOENT) 
            {
                continue;
            }
        	
        }
        
        // 新 URI 长度 = 原 URI 长度 + index 文件名长度
        uri.len = r->uri.len + len - 1;
		
		// root 是根目录长度，path.data + root 即为相对于 root 的 URI
		uri.data = path.data + root;
		
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(trace_file_fd, "uri.len=%ld\n", uri.len);
        dprintf(trace_file_fd, "uri.data=%s\n\n", uri.data);
        
        dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(trace_file_fd, "night_http_internal_redirect(r, &uri, &r->args);\n\n");
        
		//将请求重定向到找到的 index 文件 URI
		rc = night_http_internal_redirect(r, &uri, &r->args);
		
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "night_http_index_handler return rc(%d)\n\n", rc);
		
        return rc;
    }
    
    //遍历完所有 index 文件都未找到，交由后续 handler
	return NIGHT_DECLINED;
}
