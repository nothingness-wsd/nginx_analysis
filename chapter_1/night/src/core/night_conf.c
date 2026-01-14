#include "night_core.h"

static int
night_conf_add_dump(night_conf_t *cf, night_str_t *filename);

static int
night_conf_read_token(night_conf_t *cf);

static int
night_conf_handler(night_conf_t *cf, int last);


// 固定参数
static 
uint64_t argument_number[] = 
{
    NIGHT_CONF_NOARGS,
    NIGHT_CONF_TAKE1,
    NIGHT_CONF_TAKE2,
    NIGHT_CONF_TAKE3,
    NIGHT_CONF_TAKE4,
    NIGHT_CONF_TAKE5,
    NIGHT_CONF_TAKE6,
    NIGHT_CONF_TAKE7
};

char *
night_conf_parse(night_conf_t *cf, night_str_t *filename)
{
    char					*rv;
    int						fd;
    int						rc;
    night_buf_t				buf;
    night_conf_file_t		*prev, conf_file;
    
    enum 
    {
        parse_file = 0,
        parse_block,
        parse_param
    }type;

	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "%s\n\n", __func__);
    
    if (filename) 
    {
    	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd,"filename=%s\n\n", filename->data);
    	
		// open configuration file

        fd = open(filename->data, O_RDONLY, 0);
        
		if (fd == NIGHT_INVALID_FILE) 
		{
			dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(trace_file_fd,"%s open failed\n", filename->data);
			dprintf(trace_file_fd,"function %s:\t" "return NIGHT_CONF_ERROR\n\n", __func__);
            
            return NIGHT_CONF_ERROR;
        }
        
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "保存并切换配置文件上下文\n\n");
		
        prev = cf->conf_file;

        cf->conf_file = &conf_file;
        
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "获取文件元信息\n\n");
		
		if (fstat(fd, &cf->conf_file->file.info) == NIGHT_FILE_ERROR) 
		{
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(trace_file_fd, "%s fstat failed\n\n", filename->data);
		}
		
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "获取文件元信息 完成\n\n");
		
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "初始化读取缓冲区\n\n");
		
		cf->conf_file->buffer = &buf;

        buf.start = malloc(night_pagesize);
        if (buf.start == NULL) 
        {
            goto failed;
        }

        buf.pos = buf.start;
        buf.last = buf.start;
        buf.end = buf.last + night_pagesize;
        buf.temporary = 1;
        
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "初始化读取缓冲区 完成\n\n");
		
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "初始化当前正在解析的配置文件的元数据\n\n");
		
		cf->conf_file->file.fd = fd;
        cf->conf_file->file.name.len = filename->len;
        cf->conf_file->file.name.data = filename->data;
        cf->conf_file->file.offset = 0;
        cf->conf_file->line = 1;
		
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "初始化当前正在解析的配置文件的元数据 完成\n\n");
		
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "将解析模式设置为 “解析配置文件” 模式\n\n");
		
        type = parse_file;
        
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "配置转储控制\n\n");
		
		if (night_dump_config)
        {
        	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "将当前配置文件路径加入 dump 列表\n");
        	dprintf(trace_file_fd, "night_conf_add_dump(cf, %s)\n\n", filename->data);
        	
            if (night_conf_add_dump(cf, filename) != NIGHT_OK) 
            {
            	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            	dprintf(trace_file_fd, "night_conf_add_dump(cf, %s) failed\n", filename->data);
            	dprintf(trace_file_fd, "goto failed\n\n");
            	
                goto failed;
            }

        } 
        else 
        {
        	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "不启用 dump\n\n");
        	
            cf->conf_file->dump = NULL;
        }
    }
    else if (cf->conf_file->file.fd != NIGHT_INVALID_FILE) 
    {
		
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "type = parse_block\n\n");
		
        type = parse_block;

    }
    else 
    {
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "type = parse_param\n\n");
		
        type = parse_param;
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "主解析循环\n\n");
	
	for ( ;; ) 
	{
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "读取 token\n\n");
    	
		rc = night_conf_read_token(cf);
		
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "读取 token 完成\n\n");
    	
		if (rc == NIGHT_ERROR) 
		{
        	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "rc == NIGHT_ERROR\n" "读取 token 出错\n" "goto done\n\n");
        	
            goto done;
        }
        
		if (rc == NIGHT_CONF_BLOCK_DONE) 
		{
			
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(trace_file_fd, "rc == NIGHT_CONF_BLOCK_DONE\n");
			
            if (type != parse_block) 
            {   
                dprintf(trace_file_fd, "type != parse_block\n" "unexpected \"}\"\n" "goto failed\n\n");
                
                goto failed;
            }

			dprintf(trace_file_fd, "goto done\n\n");

            goto done;
        }
        
		if (rc == NIGHT_CONF_FILE_DONE) 
		{	
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(trace_file_fd, "rc == NIGHT_CONF_FILE_DONE\n");
			
            if (type == parse_block) 
            { 
                dprintf(trace_file_fd, "type == parse_block\n" "unexpected end of file, expecting \"}\"\n" "goto failed\n\n");
                
                goto failed;
            }

			dprintf(trace_file_fd, "goto done\n\n");
			
            goto done;
        }
        
		if (rc == NIGHT_CONF_BLOCK_START) 
		{
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(trace_file_fd, "rc == NIGHT_CONF_BLOCK_START\n");
			
            if (type == parse_param) 
            {
                dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
                dprintf(trace_file_fd, "type == parse_param\n" "block directives are not supported in -g option\n" "goto failed\n\n");
                
                goto failed;
            }
        }
        
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "标准指令处理器 处理指令\n" "night_conf_handler(cf, %d) \n\n", rc);
		
        rc = night_conf_handler(cf, rc);

		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "标准指令处理器 处理指令 完成\n" "night_conf_handler(cf, %d) completed\n\n", rc);
		
        if (rc == NIGHT_ERROR) 
        {
        	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "rc == NIGHT_ERROR\n" "指令处理失败\n" "goto failed\n\n");
        	
            goto failed;
        }
    }    

failed:

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "failed:\n" "rc = NIGHT_ERROR\n\n");
	
	rc = NIGHT_ERROR;
        
done:

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "done:\n");        
        
	if (filename) 
	{
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "释放缓冲区内存，关闭文件 %s 文件描述符 %d\n\n", filename->data, fd);
		
        if (cf->conf_file->buffer->start) 
        {
            free(cf->conf_file->buffer->start);
        }

        if (close(fd) == NIGHT_FILE_ERROR) 
        {

            rc = NIGHT_ERROR;
        }
		
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "恢复 cf->conf_file 到调用前的状态（支持嵌套解析）\n\n");
		
        cf->conf_file = prev;
    }

    if (rc == NIGHT_ERROR) 
    {
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "function %s:\t" "return NIGHT_CONF_ERROR\n\n", __func__);
    	
        return NIGHT_CONF_ERROR;
    }

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function %s:\t" "return NIGHT_CONF_OK\n\n", __func__);
	
    return NIGHT_CONF_OK;
        
}        

static int
night_conf_read_token(night_conf_t *cf)
{
	int   			found, need_space, last_space, sharp_comment;
	int   			quoted, s_quoted, d_quoted;
	int				variable;
    int   			start_line;
    
    night_buf_t		*b, *dump;
    night_str_t		*word;
    
	char      		*start;
	char			ch;
	char			*src, *dst;
	
    off_t        	file_size;
    size_t       	len;
    ssize_t      	n, size;
    
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "%s\n\n", __func__);
    
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"初始化局部变量\n\n");
    
	found = 0;
    need_space = 0;
    last_space = 1;
    sharp_comment = 0;
	quoted = 0;
    s_quoted = 0;
    d_quoted = 0;

    cf->args->nelts = 0;
    b = cf->conf_file->buffer;
    dump = cf->conf_file->dump;
    start = b->pos;
    start_line = cf->conf_file->line;
    
    file_size = cf->conf_file->file.info.st_size;
    
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"文件大小 file_size=%ld\n\n", file_size);
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "主循环：逐字节解析\n\n");
    
    for ( ;; ) 
    {
		if (b->pos >= b->last) 
		{
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(trace_file_fd, "当前 buffer 已读完，需要从文件加载更多内容\n\n");
			
			if (cf->conf_file->file.offset >= file_size) 
			{
            	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            	dprintf(trace_file_fd, "文件内容已全部读取完成\n\n");
            	
            	if (cf->args->nelts > 0 || !last_space) 
            	{
					dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
					dprintf(trace_file_fd, "还有未完成的 token\n\n");
					
					if (cf->conf_file->file.fd == NIGHT_INVALID_FILE) 
					{
						dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
						dprintf(trace_file_fd, "unexpected end of parameter\n" 
												"expecting \";\"" 
												"\n\n");
						
						dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    					dprintf(trace_file_fd, "参数未结束\n"
							"function %s:\t" "return NIGHT_ERROR\n\n", __func__);
    											
                        return NIGHT_ERROR;
                    }
                    
                    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
                    dprintf(trace_file_fd, "unexpected end of file,\n"
                    						"expecting \";\" or \"}\"" 
                    						"\n\n");
                    
					dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
					dprintf(trace_file_fd, "文件意外结束\n"
						"function %s:\t" "return NIGHT_ERROR\n\n", __func__);
						
                    return NIGHT_ERROR;
            	}
            	
				dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
                dprintf(trace_file_fd, "文件正常结束\n" "function %s:\t" "return NIGHT_CONF_FILE_DONE\n", __func__);
                
            	return NIGHT_CONF_FILE_DONE;
            }
            
            
			len = b->pos - start;

            if (len == night_pagesize) 
            {
            	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            	dprintf(trace_file_fd, "当前 token 长度已达 buffer 上限\n\n");
            	
            	cf->conf_file->line = start_line;
            	
				if (d_quoted) 
				{
                    ch = '"';
                } 
                else if (s_quoted) 
                {
					ch = '\'';
                } 
                else 
                {
                	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
                	dprintf(trace_file_fd, "too long parameter\n \"%s...\" started\n\n", start);
                	
                	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
                	dprintf(trace_file_fd, "function %s:\t" "return NIGHT_ERROR\n\n", __func__);
                	
                    return NIGHT_ERROR;
                }

				dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
				dprintf(trace_file_fd, "too long parameter, probably missing terminating \"%c\" character\n\n", ch);
				
				dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
				dprintf(trace_file_fd, "function %s:\t" "return NIGHT_ERROR\n\n", __func__);
                	
                return NIGHT_ERROR;
			}
			
			if (len) 
			{
            	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            	dprintf(trace_file_fd, "将未处理完的 token 数据（从 start 开始）移到 buffer 开头\n\n");
            	
                memmove(b->start, start, len);
            } 
            
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(trace_file_fd, "计算还能读多少字节（不超过 buffer 剩余空间）\n\n");
			
            size = (ssize_t) (file_size - cf->conf_file->file.offset);

            if (size > b->end - (b->start + len)) 
            {
                size = b->end - (b->start + len);
            }
            
            dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            dprintf(trace_file_fd, "还能读 size=%ld 字节\n\n", size);	
            
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(trace_file_fd, "从文件读取新数据\n" "night_read_file\n\n");
			
			n = night_read_file(&cf->conf_file->file, b->start + len, size, cf->conf_file->file.offset);
			
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(trace_file_fd, "从文件读取新数据 完成\n\n");
			
			if (n == NIGHT_ERROR) 
			{
				dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            	dprintf(trace_file_fd, "文件读取 失败\n");
            	dprintf(trace_file_fd, "function %s:\t" "return NIGHT_ERROR\n\n", __func__);
            	
                return NIGHT_ERROR;
            }
            
			if (n != size) 
			{                      
				dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            	dprintf(trace_file_fd, "文件读取 失败\n" "预想读取 %ld 字节数据，实际读取 %ld 字节数据\n", size, n);
            	dprintf(trace_file_fd, "function %s:\t" "return NIGHT_ERROR\n\n", __func__);  
            	                 
                return NIGHT_ERROR;
            }
            
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(trace_file_fd, "更新数据缓冲区信息\n\n");
			
            b->pos = b->start + len;
            b->last = b->pos + n;
            start = b->start;
            
			if (dump) 
			{	
            	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            	dprintf(trace_file_fd, "启用了 dump，复制原始内容\n\n");
            	
                dump->last = night_cpymem(dump->last, b->pos, size);
            } 
		}
		
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "解析缓冲区中的内容\n\n");
		
        ch = *b->pos++;
		
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "读取当前字符 ch=%c\n\n", ch);
		
		if (ch == LF) 
		{
            cf->conf_file->line++;

            if (sharp_comment) 
            {
                sharp_comment = 0;
            }
        }
        
		if (sharp_comment) 
		{
            continue;
        }
        
		if (quoted) 
		{
            quoted = 0;
            continue;
        }
        
        if (need_space) 
        {
			if (ch == ' ' || ch == '\t' || ch == CR || ch == LF) 
			{
                last_space = 1;
                need_space = 0;
                continue;
            }
            
			if (ch == ';') 
			{
				dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
				dprintf(trace_file_fd, "function %s\t: return NIGHT_OK\n\n", __func__);
            	
				return NIGHT_OK;
			}
			
			if (ch == '{') 
			{
				dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
				dprintf(trace_file_fd, "function %s\t: return NIGHT_CONF_BLOCK_START\n\n", __func__);
            	
				return NIGHT_CONF_BLOCK_START;
			}
			else 
			{
				dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
				dprintf(trace_file_fd, "unexpected \"%c\"\n\n", ch);
			
				dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
				dprintf(trace_file_fd, "function %s\t: return NIGHT_ERROR\n\n", __func__);
                
				return NIGHT_ERROR;
			}
        }
        
		if (last_space) 
		{
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(trace_file_fd, "开始新 token\n\n");
			
			start = b->pos - 1;
			start_line = cf->conf_file->line;
			
			if (ch == ' ' || ch == '\t' || ch == CR || ch == LF) 
			{
                continue;
            }
            
            switch (ch) 
            {
				case ';':
            	case '{':
                	if (cf->args->nelts == 0) 
                	{
                		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
                		dprintf(trace_file_fd, "unexpected \"%c\"\n" "function %s:\t" "return NIGHT_ERROR\n\n", ch, __func__);
                		
                    	return NIGHT_ERROR;
                	}

                	if (ch == '{') 
                	{
                		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
                		dprintf(trace_file_fd, "function %s:\t" "return NIGHT_CONF_BLOCK_START\n\n", __func__);
                	
                    	return NIGHT_CONF_BLOCK_START;
                	}

					dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
					dprintf(trace_file_fd, "function %s:\t" "return NIGHT_OK\n\n", __func__);
                	
                	return NIGHT_OK;
                
				case '}':
                	if (cf->args->nelts != 0) 
                	{
                    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
                    	dprintf(trace_file_fd, "unexpected \"}\"\n" "function %s:\t" "return NIGHT_ERROR\n\n", __func__);
                    
                    	return NIGHT_ERROR;
                	}

					dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
					dprintf(trace_file_fd, "function %s:\t" "return NIGHT_CONF_BLOCK_DONE\n\n", __func__);
				
                	return NIGHT_CONF_BLOCK_DONE;	
                	
				case '#':
                	sharp_comment = 1;
                	continue;
                	
				case '\\':
                	quoted = 1;
                	last_space = 0;
                	continue;
                	
				case '"':
                	start++;
                	d_quoted = 1;
                	last_space = 0;
                	continue;

            	case '\'':
                	start++;
                	s_quoted = 1;
                	last_space = 0;
                	continue; 	
                	
				case '$':
                	variable = 1;
                	last_space = 0;
                	continue;

            	default:
                	last_space = 0;	
            }
		}
		else
		{
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(trace_file_fd, "token 解析进行中\n\n");
			
			if (ch == '{' && variable) 
			{
                continue;
            }
            
            variable = 0;
            
			if (ch == '\\') 
			{
                quoted = 1;
                continue;
            }

            if (ch == '$') 
            {
                variable = 1;
                continue;
            }
            
			if (d_quoted) 
			{
                if (ch == '"') 
                {
                    d_quoted = 0;
                    need_space = 1;
                    found = 1;
                }

            } 
            else if (s_quoted) 
            {
                if (ch == '\'') 
                {
                    s_quoted = 0;
                    need_space = 1;
                    found = 1;
                }

            } 
            else if (ch == ' ' || ch == '\t' || ch == CR || ch == LF || ch == ';' || ch == '{')
            {
                last_space = 1;
                found = 1;
            }
            
            if (found) 
            {
				dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            	dprintf(trace_file_fd, "found == 1，提取 token\n\n");
            	
				word = night_array_push(cf->args);
                if (word == NULL) 
                {
                    return NIGHT_ERROR;
                }
                
				word->data = night_pnalloc(cf->pool, b->pos - 1 - start + 1);
                if (word->data == NULL) 
                {
                    return NIGHT_ERROR;
                }
                
				for (dst = word->data, src = start, len = 0;
                     src < b->pos - 1;
                     len++)
                {
                    if (*src == '\\') 
                    {
                        switch (src[1]) 
                        {
                        	case '"':
                        	case '\'':
                        	case '\\':
                            	src++;
                            	break;

                        	case 't':
                            	*dst++ = '\t';
                            	src += 2;
                            	continue;

                        	case 'r':
                            	*dst++ = '\r';
                            	src += 2;
                            	continue;

                        	case 'n':
                            	*dst++ = '\n';
                            	src += 2;
                            	continue;
                        }
                    }
                    *dst++ = *src++;
                }
                
                *dst = '\0';
                word->len = len;
                
				dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
				dprintf(trace_file_fd, "word->len=%ld\n" "word->data=%s\n\n", word->len, word->data);
				
				if (ch == ';') 
				{
                    return NIGHT_OK;
                }

                if (ch == '{') 
                {
                    return NIGHT_CONF_BLOCK_START;
                }

                found = 0;
            }
		}
    }
}

static int
night_conf_handler(night_conf_t *cf, int last)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "%s\n\n", __func__);
	
	char				*rv;
    void           		*conf, **confp;
    int					i, found;
    night_str_t			*name;
    night_command_t		*cmd;
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "提取指令名\n");

    name = cf->args->elts;
    
    dprintf(trace_file_fd, "指令名:%s\n\n", name->data);
    
    found = 0;
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "遍历所有已加载模块\n\n");
    
    for (i = 0; cf->cycle->modules[i]; i++) 
    {
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "module:%s\n\n", cf->cycle->modules[i]->name);
		
		cmd = cf->cycle->modules[i]->commands;
        if (cmd == NULL) 
        {
            continue;
        }
        
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "遍历当前模块的所有指令\n\n");
		
        for ( /* void */ ; cmd->name.len; cmd++) 
        {
            if (name->len != cmd->name.len) 
            {
                continue;
            }

            if (strcmp(name->data, cmd->name.data) != 0) 
            {
                continue;
            }

            found = 1;
            
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            dprintf(trace_file_fd, "found directive %s in module %s\n\n", name->data, cf->cycle->modules[i]->name);
            
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(trace_file_fd, "模块类型合法性检查\n\n");
			
            if (cf->cycle->modules[i]->type != NIGHT_CONF_MODULE
                && cf->cycle->modules[i]->type != cf->module_type)
            {
            	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            	dprintf(trace_file_fd, "模块类型不合法\n");
            	dprintf(trace_file_fd, "当前模块类型是:%ld\n\n", cf->module_type);
            	
                continue;
            }
            
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(trace_file_fd, "模块类型合法\n\n");
			
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            dprintf(trace_file_fd, "指令 作用域 合法性检查\n\n");

            if (!(cmd->type & cf->cmd_type)) 
            {
				dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            	dprintf(trace_file_fd, "指令 作用域 不合法\n\n");
            	
                continue;
            }
            
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            dprintf(trace_file_fd, "指令 作用域 合法性检查 完成\n\n");
            
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(trace_file_fd, "指令结束符检查\n\n");
			
            if (!(cmd->type & NIGHT_CONF_BLOCK) && last != NIGHT_OK) 
            {                     
                dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
                dprintf(trace_file_fd, "directive \"%s\" is not terminated by \";\"\n"
                	"function %s:\t" "return NIGHT_ERROR", name->data, __func__);
                                  
                return NIGHT_ERROR;
            }
            
			if ((cmd->type & NIGHT_CONF_BLOCK) && last != NIGHT_CONF_BLOCK_START) 
			{
                
                dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
                dprintf(trace_file_fd, "directive \"%s\" has no opening \"{\"\n"
                	"function %s:\t" "return NIGHT_ERROR\n\n", name->data, __func__);
                                   
                return NIGHT_ERROR;
            }
            
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(trace_file_fd, "指令结束符检查 完成\n\n");
			
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            dprintf(trace_file_fd, "参数数量检查\n\n");
            
            if (!(cmd->type & NIGHT_CONF_ANY)) 
            {
				dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
				dprintf(trace_file_fd, "指令参数非 any\n\n");
				
				if (cmd->type & NIGHT_CONF_FLAG) 
				{
                	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
                	dprintf(trace_file_fd, "布尔型指令,参数数量必须是 1\n\n");

                    if (cf->args->nelts != 2) 
                    {
                    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
                    	dprintf(trace_file_fd, "参数数量不是 1\n" "goto invalid\n\n");
                    	
                        goto invalid;
                    }
                }
                else if(cmd->type & NIGHT_CONF_1MORE)
                {
					dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
                	dprintf(trace_file_fd, "至少1个参数\n\n");
                	
					if (cf->args->nelts < 2) 
					{
						dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
                		dprintf(trace_file_fd, "参数数量不足 1\n" "goto invalid\n\n");
                	
                        goto invalid;
                    }
                }
                else if (cmd->type & NIGHT_CONF_2MORE) 
                {
					dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
                	dprintf(trace_file_fd, "至少2个参数\n\n");
                	
					if (cf->args->nelts < 3) 
					{
                    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
                    	dprintf(trace_file_fd, "参数数量不足 2\n" "goto invalid\n\n");
                    	
                        goto invalid;
                    }
                }
                else if (cf->args->nelts > NIGHT_CONF_MAX_ARGS) 
                {
					dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
                	dprintf(trace_file_fd, "参数过多\n" "goto invalid\n\n");
                	
                    goto invalid;
				}
				else if (!(cmd->type & argument_number[cf->args->nelts - 1]))
                {
                	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
                	dprintf(trace_file_fd, "固定参数数量 与 当前参数数量 不等\n" "goto invalid\n\n");
                	
                    goto invalid;
                }     
            }
            
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            dprintf(trace_file_fd, "参数数量检查 完成\n\n");
            
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(trace_file_fd, "确定配置结构体的位置\n\n");
			
			conf = NULL;
			
			if (cmd->type & NIGHT_DIRECT_CONF) 
			{
            	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            	dprintf(trace_file_fd, "DIRECT 类型指令，用于核心模块\n\n");
            	
                conf = ((void **) cf->ctx)[cf->cycle->modules[i]->index];

            } 
            else if (cmd->type & NIGHT_MAIN_CONF) 
            {
				dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            	dprintf(trace_file_fd, "MAIN 类型指令\n\n");
            	
                conf = &(((void **) cf->ctx)[cf->cycle->modules[i]->index]);

            } 
            else if (cf->ctx) 
            {
				dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            	dprintf(trace_file_fd, "其他 类型指令\n");
            	dprintf(trace_file_fd, "字段偏移量:%ld\n\n", cmd->conf);
            	
                confp = *(void **) ((char *) cf->ctx + cmd->conf);

                if (confp) 
                {
                	dprintf(trace_file_fd, "索引 ctx_index=%d\n\n", cf->cycle->modules[i]->ctx_index);
                	
                    conf = confp[cf->cycle->modules[i]->ctx_index];
                }
            }
            
            			
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(trace_file_fd, "调用指令的 set 函数（真正处理配置）\n\n"); 
			
            rv = cmd->set(cf, cmd, conf);
            
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(trace_file_fd, "指令的 set 函数 处理完成\n\n"); 
            
			if (rv == NIGHT_CONF_OK) 
			{
				dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
				dprintf(trace_file_fd, "指令的 set 函数 处理 成功\n"); 
				dprintf(trace_file_fd, "function %s:\t" "return NIGHT_OK\n\n", __func__);
			
                return NIGHT_OK;
            }

            if (rv == NIGHT_CONF_ERROR) 
            {
				dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
				dprintf(trace_file_fd, "指令的 set 函数 处理 失败\n"); 
				dprintf(trace_file_fd, "function %s:\t" "return NIGHT_ERROR\n\n", __func__);
				
                return NIGHT_ERROR;
            }

			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(trace_file_fd, "指令的 set 函数 处理 失败\n"
									"\"%s\" directive %s\n\n", name->data, rv);
					
			dprintf(trace_file_fd, "function %s:\t" "return NIGHT_ERROR\n\n", __func__);
				
            return NIGHT_ERROR;
		}
    }
    
    if (found) 
    {

		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "指令名存在，但不合法\n"
								"\"%s\" directive is not allowed here\n", name->data);
								
		dprintf(trace_file_fd, "function %s:\t" "return NIGHT_ERROR\n\n", __func__);					
		
	
        return NIGHT_ERROR;
    }
    
                       
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "未知指令 %s\n" "function %s:\t" "return NIGHT_ERROR\n\n", name->data, __func__);		

    return NIGHT_ERROR;
    
invalid:

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "invalid number of arguments in \"%s\" directive\n"
							"function %s:\t" "return NIGHT_ERROR\n\n", name->data, __func__);	
	
    return NIGHT_ERROR;
}

static int
night_conf_add_dump(night_conf_t *cf, night_str_t *filename)
{

}

