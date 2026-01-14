#include "night_core.h"
#include "night_conf.h"
#include "night_conf_file.h"
#include "night_pool.h"
#include "night_module.h"
#include "night_cycle.h"
#include "night_string.h"

int
night_conf_parse(night_conf_t *cf, night_str_t *filename)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_conf_parse\n\n");
    
    int 				conf_fd;
    night_conf_file_t 	conf_file;
    int 				rc;
    
    enum 
    {
        parse_file = 0,
        parse_block
    } type;
    
    rc = NIGHT_OK;
    
    // parse file
    if (filename && filename->len)
    {
        type = parse_file;
        conf_fd = open(filename->data, O_RDONLY, 0 );
        
        // init conf_file
        cf->conf_file = &conf_file;
        
        // init conf_file.file
        // get file state info
        if(fstat(conf_fd, &conf_file.file.info) < 0)
        {
        	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);;
            dprintf(error_log_fd, "fstat() to get configuration file state info failed\nerrno=%d:%s\n\n",errno,strerror(errno));
            
            return NIGHT_ERROR;
        }
        
        conf_file.file.filename.len = filename->len;
        conf_file.file.filename.data = night_pmalloc(cf->pool,filename->len + 1);
        
        memcpy(conf_file.file.filename.data, filename->data, filename->len);
        
        dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(trace_file_fd,"conf_file.file.filename.len=%ld\n", conf_file.file.filename.len);
        dprintf(trace_file_fd,"conf_file.file.filename.data=%s\n\n", conf_file.file.filename.data);
        
        conf_file.file.fd = conf_fd;
        conf_file.file.offset = 0;
        
        // init conf_file.buffer
        conf_file.buffer.start = (char*) night_pmalloc(cf->pool, NIGHT_DEFAULT_POOL_LIMIT);
        conf_file.buffer.pos = conf_file.buffer.start;
        conf_file.buffer.last = conf_file.buffer.start;
        conf_file.buffer.end = conf_file.buffer.last + NIGHT_DEFAULT_POOL_LIMIT;
        
        // line of current position at configuration file      
        conf_file.line = 1;
    }
    // parse configuration block
    else
    {
        type = parse_block;
    }
    
    // read and handle token 
    for( ; ; )
    {
        dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(trace_file_fd,"rc = night_conf_read_token(cf)\n\n");
        
        rc = night_conf_read_token(cf);
        if(rc == NIGHT_ERROR) 
        {
            goto done;
        }
        
        if( rc == NIGHT_CONF_BLOCK_DONE )
        {
            if(type != parse_block)
            {
            	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
                dprintf(error_log_fd,"unexpected \"}\" at line %ld\n\n", conf_file.line);
            }
            goto done;
        }
        
        if(rc == NIGHT_CONF_FILE_DONE)
        {
            if( type != parse_file)
            {
            	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
                dprintf(error_log_fd,"unexpected end of file, expecting \"}\" at line %ld\n\n",conf_file.line);
            }
            goto done;
        }
        
        // NIGHT_CONF_BLOCK_START or NIGHT_CONF_OK
        rc = night_conf_handler(cf, rc);
        if (rc == NIGHT_ERROR) 
        {
            goto done;
        }
    }
    
done:
    if(filename && filename->len)
    {
        close(conf_file.file.fd );
    }
    
    if( rc == NIGHT_ERROR)
    {
        return NIGHT_ERROR;
    }   
     
    return NIGHT_OK;
}

int 
night_conf_read_token(night_conf_t *cf)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_conf_read_token\n\n");
    
    night_buf_t	*b;
    size_t 		file_size;
    size_t 		size;
    ssize_t 	n;
    char 		*word_start;
    int 		last_space;
    size_t 		len;
    char 		ch;
    int 		sharp_comment;
    int 		need_space;
    size_t 		word_start_line;
    int 		found;
    night_str_t *word;
    char		*dst;
    char		*src;
    size_t 		word_len; 
    
    // Buffer
    b = &cf->conf_file->buffer;
    
    // init
    cf->args.nelts = 0;
    word_start = b->pos;
    last_space = 1;
    sharp_comment = 0;
    need_space = 0;
    word_start_line = 0;
    found = 0;
    
    // Get file size
    file_size = cf->conf_file->file.info.st_size;
    
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"file_size=%ld\n\n",file_size);
    
    // Loop through read and parse each character
    for( ; ; )
    {
        // The valid data has been processed, and more data needs to be read
        if(b->pos >= b->last)
        {   
            dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            dprintf(trace_file_fd,"b->pos >= b->last\n\n");
            
            // There is no unread data in the file
            if (cf->conf_file->file.offset >= file_size) 
            {
                if (cf->args.nelts > 0 || !last_space) 
                {
                	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
                    dprintf(error_log_fd,"unexpected end of file," "expecting \";\" or \"}\"" "\n\n");
                    
                    return NIGHT_ERROR;
                }
                
                dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
                dprintf(trace_file_fd,"NIGHT_CONF_FILE_DONE\n\n");
                
                return NIGHT_CONF_FILE_DONE;
            }
            
            // The length of the unprocessed word
            len = b->pos - word_start;
            
            dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            dprintf(trace_file_fd,"len=%ld\n\n",len);
            
            if (len == (b->end - b->start)) 
            {
            	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
                dprintf(error_log_fd,"too long token at line %ld\n\n", cf->conf_file->line);
                
                return NIGHT_ERROR;
	    	}
	    
	    	if (len) 
	    	{
                memcpy(b->start, word_start, len);
            }
            
            // There is unread data in the file
            // The size to be read
            size = file_size - cf->conf_file->file.offset;
            if(size > (b->end - b->start - len))
            {
                size = b->end - b->start - len;
            }
            
            // Read configuration file
            n = night_read_file( &cf->conf_file->file, b->start + len, size, cf->conf_file->file.offset);
        
            dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            dprintf(trace_file_fd,"n=%ld\n\n" , n);
        
            if(n < 0)
            {
            	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
                dprintf(error_log_fd, "night_read_file failed to read configuration file\n"
                						"errno=%d:%s\n\n", errno, strerror(errno));
                
                return NIGHT_ERROR;
            }
            
            // Updata buffer
            b->pos = b->start + len;
            b->last = b->pos + n;
            
            // The start of word 
            word_start = b->start;
        }
        
        // Parse each character
        ch = *b->pos++;
        
        dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(trace_file_fd,"ch=%c\n\n",ch);

        if(ch == LF)
        {
            if(sharp_comment) 
            {
                sharp_comment = 0;
            }
            cf->conf_file->line++;
        }
        
        if(sharp_comment) 
        {
            continue;
        }    
        
        if(need_space)
        {
            if (ch == ' ' || ch == '\t' || ch == CR || ch == LF) 
            {
                last_space = 1;
                need_space = 0;
                
                continue;
            }
            
            if (ch == ';') 
            {
                return NIGHT_OK;
            }
            
            if(ch == '{')
            {
                return NIGHT_CONF_BLOCK_START;
            }
            else
            {
            	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
                dprintf(error_log_fd,"unexpected \"%c\" (need space) at line %ld\n\n", ch, cf->conf_file->line);
                
                return NIGHT_ERROR;
            }
        }
        
        if(last_space) 
        {
            if(ch == ' ' || ch == '\t' || ch == CR || ch == LF) 
            {   
                continue;
            }
            
            word_start = b->pos - 1;
            word_start_line = cf->conf_file->line;
            
            switch(ch) 
            {
                case ';':
                case '{':
                    if(cf->args.nelts == 0) 
                    {
                    	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
                        dprintf(error_log_fd,"unexpected \"%c\" (need argument) at line %ld\n\n", ch, cf->conf_file->line);
                        
                        return NIGHT_ERROR;
                    }
                    
                    if (ch == '{') 
                    {
                    	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
                    	dprintf(trace_file_fd,"NIGHT_CONF_BLOCK_START\n\n");
                    	
                        return NIGHT_CONF_BLOCK_START;
                    }
                    
                    return NIGHT_CONF_OK;
                    
                case '}':
                    if (cf->args.nelts != 0)
                    {
                    	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
                        dprintf(error_log_fd,"unexpected \"}\" (need \';\') at line %ld\n\n", cf->conf_file->line);
                        
                        return NIGHT_ERROR;
                    }
                    
                    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
                    dprintf(trace_file_fd,"NIGHT_CONF_BLOCK_DONE\n\n");
                    
                    return NIGHT_CONF_BLOCK_DONE;
                
                case '#':
                    sharp_comment = 1;
                    continue;
                    
                default:
                    last_space = 0;
                    break;    
            }
        }
        else
        {
            if (ch == ' ' || ch == '\t' || ch == CR || ch == LF || ch == ';' || ch == '{')
            {
                last_space = 1;
                found = 1;
            }
            
            if(found) 
            {
                word = (night_str_t*) night_array_push(&cf->args);
                if (word == NULL)
                {
                	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
                    dprintf(error_log_fd,"night_array_push failed to get a new args's element while read token\n\n");
                    
                    return NIGHT_ERROR;
                }
                
                word->data = night_pmalloc(cf->pool, b->pos - word_start);
                if(word->data == NULL)
                {
                	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
                    dprintf(error_log_fd,"night_pmalloc failed to allocate memory for word while read token\n\n");
                    
                    return NIGHT_ERROR;
                }
                
                for (dst = word->data, src = word_start, word_len = 0; src < b->pos - 1; word_len++)
                {
                    *dst++ = *src++;
                }
                *dst = '\0';
                word->len = word_len;
                
                dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
                dprintf(trace_file_fd,"word->len=%ld\n", word->len);
                dprintf(trace_file_fd,"word->data=%s\n\n", word->data);
                
                if(ch == ';') 
                {
                    return NIGHT_CONF_OK;
                }
                
                if(ch == '{')
                {
                    return NIGHT_CONF_BLOCK_START;
                }
                
                found = 0;
            }
        }
    }
    
    return NIGHT_OK;
}

int
night_conf_handler(night_conf_t *cf, int last)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_conf_handler\n\n");
    
    int 			i;
    night_command_t	*cmd;
    night_str_t		*name;
    int 			found;
    void			*conf;
    void			**confp;
    int 			rc;
    
    name = cf->args.elts;
    found = 0;
    
    for(i = 0; cf->cycle->modules[i]; i++) 
    {
        dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(trace_file_fd,"modules[%d]->name=%s\n\n" , i, cf->cycle->modules[i]->name);
        
        cmd = cf->cycle->modules[i]->commands;
        if(cmd == NULL)
        {
            continue;
        }
        
        for( ; cmd->name.len; cmd++)
        {
            if(name->len != cmd->name.len)
            {
                continue;
            }
            
            if(strcmp(name->data,cmd->name.data) != 0)
            {
                continue;
            }
            
            found = 1;
            
            conf = NULL;
            
            if(cmd->type & NIGHT_DIRECT_CONF) 
            {
                conf = cf->ctx[cf->cycle->modules[i]->index];
                
                dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
                dprintf(trace_file_fd,"if(cmd->type & NIGHT_DIRECT_CONF)\n\n");
            }
            else if(cmd->type & NIGHT_MAIN_CONF)
            {
                conf = &(cf->ctx[cf->cycle->modules[i]->index]);
                
                dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
                dprintf(trace_file_fd,"else if(cmd->type & NIGHT_MAIN_CONF)\n\n");
            }
            else if(cf->ctx)
            {
                confp = *(void***) (((char*) (cf->ctx)) + (cmd->offset));
                if(confp)
                {
                    conf = confp[cf->cycle->modules[i]->ctx_index];
                }
                
                dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
                dprintf(trace_file_fd,"else if(cf->ctx)\n\n");
            }
            
            rc = cmd->set(cf, cmd, conf);
            return rc; 
        }
    }
    
    dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(error_log_fd,"unknown directive \"%s\"\n\n", name->data);

    return NIGHT_ERROR;
}

int
night_conf_set_num(night_conf_t *cf, night_command_t *cmd, void *conf)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_conf_set_num\n\n");
    
	char 		*p;
    int 		*np;
    night_str_t *value;
    //ngx_conf_post_t  *post;

	p = (char*) conf;

    np = (int*) (p + cmd->offset);
	value = cf->args.elts;
	
    if (*np != NIGHT_CONF_INT_UNSET) 
    {
    	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd,"%s is duplicate\n\n", value[0].data);
    	
        return NIGHT_OK;
    }

    *np = night_atoi(value[1].data, value[1].len);
    if (*np == NIGHT_ERROR) 
    {
		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd,"%s is invalid number\n\n", value[0].data);
    	
        return NIGHT_OK;
    }

    return NIGHT_OK;
}
