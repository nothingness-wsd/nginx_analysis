#include "night_core.h"
#include "night_output_chain.h"
#include "night_buf.h"
#include "night_file.h"

int
night_output_chain(night_output_chain_ctx_t *ctx, night_chain_t *in)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_output_chain\n\n");
	
	night_chain_t  	*cl, *out, **last_out;
	int				last;
	size_t			bsize;
	int				rc;
	
	// 检查上下文是否为空闲状态
	if (ctx->in == NULL)
	{
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "ctx->in=%p\n\n", ctx->in);
	}
	
	// 如果输入链不为空，将其复制到上下文的输入链中，失败则返回错误
    if (in) 
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "add the incoming buf to the chain ctx->in\n\n");
    	
        if (night_output_chain_add_copy(ctx->pool, &ctx->in, in) == NIGHT_ERROR) 
        {
            return NIGHT_ERROR;
        }
    }
    
	// 初始化输出链相关变量
    out = NULL;
    last_out = &out;
    last = NIGHT_NONE;
    
	// 无限循环处理输入链中的缓冲区
    for ( ;; ) 
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "for ( ;; )\n\n");
    	
		// 循环处理上下文输入链中的所有缓冲区
        while (ctx->in) 
        {
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(trace_file_fd, "while (ctx->in)\n\n");
			
			// 获取当前缓冲区的大小
            bsize = night_buf_size(ctx->in->buf);
                        
            dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            dprintf(trace_file_fd, "bsize=%ld\n\n", bsize);
            
			//如果当前没有可用的工作缓冲区
            if (ctx->buf == NULL) 
            {
				dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
				dprintf(trace_file_fd, "if (ctx->buf == NULL)\n\n");
				
				//尝试对文件缓冲区进行对齐处理
                rc = night_output_chain_align_file_buf(ctx, bsize);
                
				if (rc == NIGHT_ERROR) 
				{
                    return NIGHT_ERROR;
                }
                
				if (rc != NIGHT_OK) 
				{
					dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
					dprintf(trace_file_fd, "if (rc != NIGHT_OK)\n\n");
				}	
				
				//检查系统中是否有之前使用过但已释放的缓冲区链表节点
				//这些节点的内存仍然有效，可以被重新使用
				if (ctx->free) 
				{ 
					dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
					dprintf(trace_file_fd, "if (ctx->free)\n\n");
                } 
                // 分配新的缓冲区 
                else if (night_output_chain_get_buf(ctx, bsize) != NIGHT_OK) 
                {
					dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
					dprintf(trace_file_fd, "return NIGHT_ERROR;\n\n");
                    	
					return NIGHT_ERROR;
				}  	
			}
			
			// 将输入缓冲区的数据复制到工作缓冲区
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(trace_file_fd, "rc = night_output_chain_copy_buf(ctx);\n\n");
			
            rc = night_output_chain_copy_buf(ctx);
            
			if (rc == NIGHT_ERROR) 
			{
                return rc;
            }
			if (rc == NIGHT_AGAIN) 
			{
            	 //如果已有输出数据，先发送已处理的数据
                if (out) 
                {
                    break;
                }
				// 如果没有输出数据，返回继续等待
                return rc;
            }
            
			// 如果当前缓冲区已完全处理，从输入链中移除
            if (night_buf_size(ctx->in->buf) == 0) 
            {	
            	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            	dprintf(trace_file_fd, "if (night_buf_size(ctx->in->buf) == 0)\n");
            	dprintf(trace_file_fd, "ctx->in = ctx->in->next;\n\n");
            	
                ctx->in = ctx->in->next;
            }
            
			// 创建新的链表节点，将处理后的缓冲区添加到输出链，并重置工作缓冲区指针
            cl = night_pmalloc_chain_link(ctx->pool);
            if (cl == NULL) 
            {
                return NIGHT_ERROR;
            }

            cl->buf = ctx->buf;
            cl->next = NULL;
            *last_out = cl;
            last_out = &cl->next;
            ctx->buf = NULL;
		}
		
		// out == NULL：输出链为空，表示当前没有可发送的数据 
		// last != NGX_NONE：之前有过过滤器调用（last 不是初始值 NGX_NONE）
		// 这个条件表示：没有新数据可发送，但之前的处理有结果
        if (out == NULL && last != NIGHT_NONE) 
        {	
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(trace_file_fd, "if (out == NULL && last != NIGHT_NONE)\n\n");
			
			// 如果输入链还有数据待处理：
			// ctx->in 不为空表示还有缓冲区需要处理
			// 返回 NGX_AGAIN 表示需要等待（可能需要更多缓冲区空间或其他资源）
			// 这里暗示当前无法继续处理输入数据，需要暂停
            if (ctx->in) 
            {
                return NIGHT_AGAIN;
            }
			
			// 如果输入链也为空了，返回之前过滤器的处理结果
			// last 包含了最后一次调用过滤器的返回值
			
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(trace_file_fd, "return last(%d);\n\n", last);
			
            return last;
        }
        
		// 调用输出过滤器处理输出链，如果返回错误或完成状态则返回
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "last = ctx->output_filter(ctx->filter_ctx, out);\n\n");
		
        last = ctx->output_filter(ctx->filter_ctx, out);

        if (last == NIGHT_ERROR || last == NIGHT_DONE) 
        {
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "return last(%d);\n\n", last);
        	
            return last;
        }
        
		// 更新链表状态：将处理过的缓冲区移动到空闲链表或繁忙链表，重置输出链指针，继续循环处理剩余数据
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "night_chain_update_chains\n\n");
		
        night_chain_update_chains(ctx->pool, &ctx->free, &ctx->busy, &out, ctx->tag);
        last_out = &out;
        
    }	
}	

/*
函数的作用是将输入的缓冲链（in）复制到输出链（chain）中，创建新的链节点而不共享原始缓冲区，确保数据的独立性。
*/
int
night_output_chain_add_copy(night_pool_t *pool, night_chain_t **chain, night_chain_t *in)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_output_chain_add_copy\n\n");
	
	night_chain_t  *cl, **ll;
	
	ll = chain;
	
	// 遍历现有链表：找到当前输出链的最后一个节点
    for (cl = *chain; cl; cl = cl->next) 
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "ll = &cl->next;\n\n");
    	
        ll = &cl->next;
    }
    
    // 遍历输入链：处理输入链中的每个节点
    while (in) 
    {
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "while (in)\n\n");
		
		//从内存池分配一个新的链节点
		//如果分配失败，返回错误	
		cl = night_pmalloc_chain_link(pool);
        if (cl == NULL) 
        {
            return NIGHT_ERROR;
        }
        
		cl->buf = in->buf;
        in = in->next;
        
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(trace_file_fd, "cl->buf->in_file=%d\n\n", cl->buf->in_file);
        
		cl->next = NULL;
        *ll = cl;
        ll = &cl->next;
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "return NIGHT_OK;\n\n");
	
    return NIGHT_OK;
}	

int
night_output_chain_align_file_buf(night_output_chain_ctx_t *ctx, off_t bsize)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_output_chain_align_file_buf\n\n");

    night_buf_t  	*in;

    in = ctx->in->buf;
    
	//检查是否有文件关联，且文件是否启用了直接I/O
	//如果没有文件或未启用直接I/O，则返回NGX_DECLINED（不处理）
    if (in->file == NULL || !0) 
    {
    	
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "return NIGHT_DECLINED;\n\n");
    	
		// 检查当前输入缓冲区是否是链中的最后一个缓冲区
    	if (in->last_in_chain) 
    	{
    		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    		dprintf(trace_file_fd, "if (in->last_in_chain)\n\n");
    	}
    	
        return NIGHT_DECLINED;
    }
}

/*
这个函数用于在输出链处理过程中获取一个临时缓冲区，用于存储待输出的数据。
*/
int
night_output_chain_get_buf(night_output_chain_ctx_t *ctx, off_t bsize)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_output_chain_get_buf\n\n");
	
	night_buf_t  		*in;
	size_t       		size;
    night_buf_t  	 	*b;
    int   				recycled;

    in = ctx->in->buf;
    //size = ctx->bufs.size;
    //recycled = 1;

	// 检查当前输入缓冲区是否是链中的最后一个缓冲区
    if (in->last_in_chain) 
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "if (in->last_in_chain)\n\n");
    	
    	// 如果需要的大小小于标准缓冲区大小
        if (bsize < (off_t) size) 
        {
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(trace_file_fd, "if (bsize < (off_t) size)\n\n");
			
			// 将实际分配大小设置为需要的大小
			// 将 recycled 设置为 0，表示这个小缓冲区不回收（因为太小，回收意义不大）

            size = (size_t) bsize;
            recycled = 0;
		}	
    }
    
	b = night_pmalloc_buf(ctx->pool);
    if (b == NULL) 
    {
        return NIGHT_ERROR;
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "else\n\n");
    	
	b->start = night_pmalloc(ctx->pool, size);
	if (b->start == NULL) 
	{
		return NIGHT_ERROR;
	}	
	
	b->pos = b->start;
    b->last = b->start;
    b->end = b->last + size;
    //b->temporary = 1;
    b->tag = ctx->tag;
    b->recycled = recycled;

    ctx->buf = b;
    //ctx->allocated++;

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "return NIGHT_OK;\n\n");
	
    return NIGHT_OK;
}	

/*
将输入缓冲区链中的数据复制到输出缓冲区中，支持内存缓冲区和文件缓冲区的不同处理方式。
*/
int
night_output_chain_copy_buf(night_output_chain_ctx_t *ctx)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_output_chain_copy_buf\n\n");
	
	off_t        	size;
    ssize_t      	n;
    night_buf_t   	*src, *dst;

    src = ctx->in->buf;
    dst = ctx->buf;
    
	//计算源缓冲区中待处理数据的大小
	//限制复制大小不超过目标缓冲区的剩余空间
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	
    size = night_buf_size(src);
    
    dprintf(trace_file_fd, "size=%ld\n", size);
    
    size = night_min(size, dst->end - dst->pos);
    
    dprintf(trace_file_fd, "size=%ld\n\n", size);
    
    if (!src->in_file)
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "in memory\n\n");
    }
    else
    {
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "in file\n\n");
    	
		// 读取文件
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "n = night_read_file\n\n");
        	
		n = night_read_file(src->file, dst->pos, (size_t) size, src->file_pos);
		
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "n=%ld\n\n", n);
    
		if (n == NIGHT_ERROR) 
		{
			return (int) n;
		}

		if (n != size) 
		{
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(trace_file_fd, "read only %ld from %ld in %s\n\n", n, size, src->file->filename.data);
		
			return NIGHT_ERROR;
		}
	
		dst->last += n;
	
		dst->in_file = 0;
	
		src->file_pos += n;
	
		// 当文件读取完毕时，传递缓冲区标志位
		if (src->file_pos == src->file_last) 
		{ 
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(trace_file_fd, "if (src->file_pos == src->file_last)\n\n");
        
        	// flush 标志指示数据是否需要立即发送，不能被缓存
            dst->flush = src->flush;
            //last_buf 标志指示这是否是整个响应内容的最后一个缓冲区
            dst->last_buf = src->last_buf;
            //传递链中最后一个缓冲区标志
            dst->last_in_chain = src->last_in_chain;
		}
	} 
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "return NIGHT_OK;\n\n");
	
	return NIGHT_OK;        
}	
