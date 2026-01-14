#include "night_core.h"
#include "night_open_file_cache.h"
#include "night_file.h"

//其作用是 打开一个文件，并利用 Nginx 的文件缓存机制（open file cache）来避免重复打开和 stat 系统调用，从而提升性能。
//它支持缓存文件描述符（fd）、文件元信息（如 mtime、size、inode 等），并处理缓存过期、失效、事件监控（如 inotify）等逻辑
//功能：尝试从缓存中打开文件；若缓存未命中或失效，则实际打开文件并更新缓存。
//参数：
//cache：指向文件缓存结构（ngx_open_file_cache_t）。若为 NULL，表示不使用缓存。
//name：要打开的文件路径（ngx_str_t 类型）。
//of：输入/输出结构，用于传递配置（如是否只测试、缓存有效期等）并返回结果（fd、错误码、文件属性等）。
//pool：内存池，用于分配临时资源和注册清理回调。
int
night_open_cached_file(night_open_file_cache_t *cache, night_str_t *name, night_open_file_info_t *of, night_pool_t *pool)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_open_cached_file\n\n");
	
	struct stat                 fi;
	int							rc;
	
	// 初始化 of 结构：fd 设为无效值（-1），错误码清零
	of->fd = NIGHT_INVALID_FILE;
    of->err = 0;
    
	// 不使用缓存
    if (cache == NULL) 
    {
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "if (cache == NULL)\n\n");
    	
		// 仅测试文件是否存在（test_only）
        if (of->test_only) 
        {
        	
        	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "if (of->test_only)\n\n");
			
			//如果 of->test_only == 1，不打开文件，只调用 stat（封装在 ngx_file_info_wrapper 中）获取文件属性。
			//成功后填充 of 的各种字段（唯一标识 uniq（通常是 inode + device）、修改时间、大小、类型等）。
			//返回 NGX_OK，不缓存，因为 cache == NULL。
			//ngx_file_uniq() 通常返回 (fi.st_dev << 32) | fi.st_ino，用于唯一标识一个文件。 
            if (night_file_info_wrapper(name, of, &fi) == NIGHT_FILE_ERROR)
            {
                return NIGHT_ERROR;
            }
			
			// 提取文件的唯一标识符（uniqueidentifier），用于判断文件是否被替换或修改
            of->uniq = night_file_uniq(&fi);
            
			//获取文件的最后时间（修改时间）
            of->mtime = night_file_mtime(&fi);
            
			// 获取文件的字节大小
            of->size = night_file_size(&fi);
            
            // 获取文件在文件系统上实际占用的磁盘空间（以字节为单位）
            //of->fs_size = night_file_fs_size(&fi);
            
			//判断该路径是否为目录
            of->is_dir = night_is_dir(&fi);
            
			//判断该路径是否为普通文件
            of->is_file = night_is_file(&fi);
            
			//判断该路径是否为符号链接（symbolic link）
            of->is_link = night_is_link(&fi);
            
            // 判断该文件是否具有执行权限
            of->is_exec = night_is_exec(&fi);

			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(trace_file_fd, "return NIGHT_OK;\n\n");
			
            return NIGHT_OK;
        }
        
		// 真正打开文件（无缓存）
		// 尝试打开文件并 fstat() 获取信息
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "night_open_and_stat_file\n\n");
		
        rc = night_open_and_stat_file(name, of);
        
		if (rc == NIGHT_OK && !of->is_dir) 
		{
        
        	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "if (rc == NIGHT_OK && !of->is_dir)\n\n");
        }

		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "return rc(%d)\n\n", rc);
		
        return rc;
    }
    
	return NIGHT_OK;
}

//用于获取文件信息（类似 POSIX 的 stat() 系统调用）
//参数说明：
//name：指向文件路径的字符串（ngx_str_t 类型）。
//of：文件打开信息结构体（ngx_open_file_info_t），包含如是否禁用符号链接等配置。
//fi：用于接收文件信息的输出缓冲区（通常是 struct stat 的封装）。
//返回值：ngx_int_t，成功返回非负值（通常为 0），失败返回 NGX_FILE_ERROR。
int
night_file_info_wrapper(night_str_t *name, night_open_file_info_t *of, struct stat *fi)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_file_info_wrapper\n\n");
	
    int  				rc;	
    int  				fd;
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "name->data=%s\n\n", name->data);
    
	//获取文件信息
	rc = stat(name->data, fi);
	
	//将系统错误码保存到 of->err
	//记录失败的操作名称（ngx_file_info_n 通常是字符串 "stat"）到 of->failed
	//返回 NGX_FILE_ERROR 表示失败
	if (rc == NIGHT_FILE_ERROR) 
	{
		of->err = errno;
		of->failed = night_file_info_n;
		
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "errno=%d\n:%s\n", errno, strerror(errno));
		dprintf(trace_file_fd, "return NIGHT_FILE_ERROR;\n\n");
		
		return NIGHT_FILE_ERROR;
	}

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "return rc(%d)\n\n", rc);
		
	return rc;
}    

/*
这段代码是 Nginx 1.24.0 中 ngx_open_and_stat_file() 函数的实现，用于打开并获取文件（或目录）的元信息（如大小、修改时间、是否为目录等），同时支持缓存文件描述符、避免重复打开、支持 direct I/O、预读（read-ahead）等高级特性。该函数广泛用于 Nginx 的静态文件服务、日志写入、配置加载等场景。
参数：
name：要打开的文件路径（ngx_str_t 类型，Nginx 的字符串结构）。
of：指向 ngx_open_file_info_t 结构的指针，用于传入配置（如是否启用 directio、read_ahead 等）并返回文件信息（fd、mtime、size 等）
*/
int
night_open_and_stat_file(night_str_t *name, night_open_file_info_t *of)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_open_and_stat_file\n\n");
	
	int					fd;
	struct stat  		fi;
/*
检查是否已缓存有效的文件描述符（即之前已打开过该文件）。
NGX_INVALID_FILE 通常是 -1，表示无效 fd。
如果 of->fd 有效，说明可能可以复用，避免重复 open。
*/
    if (of->fd != NIGHT_INVALID_FILE) 
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "if (of->fd != NIGHT_INVALID_FILE)\n\n");
    }
    
/*
调用 ngx_open_file_wrapper() 打开文件：
模式：只读（NGX_FILE_RDONLY）+ 非阻塞（NGX_FILE_NONBLOCK）
操作：NGX_FILE_OPEN（仅打开，不创建）
权限：0（不适用，因为是只读）
适用于静态文件服务（如 HTML、图片等）
*/
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "night_open_file_wrapper\n\n");
	
	fd = night_open_file_wrapper(name, of, O_RDONLY|O_NONBLOCK, 0, 0);	
    
    // 如果打开失败（如权限不足、路径不存在等），标记 fd 无效并返回错误
	if (fd == NIGHT_INVALID_FILE) 
	{
        of->fd = NIGHT_INVALID_FILE;
        return NIGHT_ERROR;
    }
    
	// 成功打开后，通过 fstat(fd, &fi) 获取文件信息（比 stat 更可靠，因为 fd 已打开）。
    if (fstat(fd, &fi) == NIGHT_ERROR) 
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "%s fstat() failed", name->data);


        if (close(fd) == NIGHT_ERROR) 
        {
        	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "%s close failed\n\n", name->data);
        }

        of->fd = NIGHT_INVALID_FILE;

        return NIGHT_ERROR;
    }
    
    // 检查是否为目录
    if (night_is_dir(&fi)) 
    {
	}
	else
	{
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "else\n\n");
		
		// 如果是普通文件（非目录），则将有效 fd 保存到 of->fd，供后续读取使用。
        of->fd = fd;
	}
	
done:

	// 提取文件的唯一标识符（uniqueidentifier），用于判断文件是否被替换或修改
	of->uniq = night_file_uniq(&fi);
	
	//获取文件的最后时间（修改时间）
	of->mtime = night_file_mtime(&fi);
            
	// 获取文件的字节大小
	of->size = night_file_size(&fi);
	
	// 获取文件在文件系统上实际占用的磁盘空间（以字节为单位）
	//of->fs_size = night_file_fs_size(&fi);
            
	//判断该路径是否为目录
	of->is_dir = night_is_dir(&fi);
            
	//判断该路径是否为普通文件
	of->is_file = night_is_file(&fi);
            
	//判断该路径是否为符号链接（symbolic link）
	of->is_link = night_is_link(&fi);
            
	// 判断该文件是否具有执行权限
	of->is_exec = night_is_exec(&fi);
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "return NIGHT_OK;\n\n");
	
	return NIGHT_OK;
}

/*
用于打开文件的函数
返回值：文件描述符（成功时为非负整数，失败时为 NGX_INVALID_FILE）。
参数：
name：要打开的文件路径（ngx_str_t 类型，包含 data 和 len）。
of：文件打开信息结构体
mode：打开模式（如只读、只写等，对应 O_RDONLY 等）。
create：创建标志（如是否创建文件、是否截断等）。
access：文件权限（如 0644）。
*/
int
night_open_file_wrapper(night_str_t *name, night_open_file_info_t *of, int mode, int create, int access)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_open_file_wrapper\n\n");
	
    int  fd;
    
	fd = night_open_file(name->data, mode, create, access);

	if (fd == NIGHT_INVALID_FILE) 
	{
		of->err = errno;
		of->failed = night_open_file_n;
		return NIGHT_INVALID_FILE;
	}
		
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "return fd(%d)\n\n", fd);
		
	return fd;
}    

