#include "night_core.h"
#include "night_buf.h"
#include "night_pool.h"

night_buf_t*
night_create_temp_buf(night_pool_t *pool, size_t size)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"function:\t" "night_create_temp_buf\n\n" );
    
    night_buf_t *b;
	
    b = night_pmalloc(pool, sizeof(night_buf_t));
    if (b == NULL) 
    {
        return NULL;
    }

    b->start = night_pmalloc(pool, size);
    if (b->start == NULL) 
    {
        return NULL;
    }

    b->pos = b->start;
    b->last = b->start;
    b->end = b->last + size;
    b->temporary = 1;

    return b;
}

night_chain_t *
night_pmalloc_chain_link(night_pool_t *pool)
{
    night_chain_t  *cl;

    cl = pool->chain;

    if (cl) 
    {
        pool->chain = cl->next;
        return cl;
    }

    cl = night_pmalloc(pool, sizeof(night_chain_t));
    if (cl == NULL) 
    {
        return NULL;
    }

    return cl;
}

night_chain_t *
night_chain_update_sent(night_chain_t *in, off_t sent)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
	dprintf(trace_file_fd, "function:\t" "night_chain_update_sent\n\n");

    size_t  size;

    for ( /* void */ ; in; in = in->next) 
    {
		// 如果已发送字节数为0，说明不需要更新任何缓冲区
		// 直接跳出循环
        if (sent == 0) 
        {
            break;
        }

        size = night_buf_size(in->buf);

        if (sent >= size) 
        {
            sent -= size;

            if (in->buf->in_file) 
            {
                in->buf->file_pos = in->buf->file_last;
            }
            else
            {
            	in->buf->pos = in->buf->last;
            }

            continue;
        }

        if (in->buf->in_file) 
        {
            in->buf->file_pos += sent;
        }
        else
        {
        	in->buf->pos += (size_t) sent;
        }

        break;
    }

    return in;
}

/*
这个函数用于管理缓冲链表，将输出链表移动到忙碌链表，然后处理忙碌链表中的缓冲区：释放不需要的缓冲区到空闲链表，清理已使用完的缓冲区。
*/
void
night_chain_update_chains(night_pool_t *p, night_chain_t **free, night_chain_t **busy, night_chain_t **out, void* tag)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_chain_update_chains\n\n");
	
    night_chain_t  *cl;

	// 检查输出链表是否非空
    if (*out) 
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "if (*out)\n\n");
    	
    	// 如果忙碌链表为空，直接将输出链表赋值给忙碌链表
        if (*busy == NULL) {
        	
        	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "if (*busy == NULL)\n*busy = *out;\n\n");
        	
            *busy = *out;

        } 
        else 
        {
        	// 如果忙碌链表不为空，遍历到忙碌链表的末尾
			// 循环条件cl->next确保找到最后一个节点
        	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "for (cl = *busy; cl->next; cl = cl->next) { /* void */ }\ncl->next = *out;\n\n");
        	
            for (cl = *busy; cl->next; cl = cl->next) { /* void */ }

			// 将输出链表连接到忙碌链表的末尾
            cl->next = *out;
        }

		// 清空输出链表指针，因为内容已经移动到忙碌链表
        *out = NULL;
    }

	// 开始处理忙碌链表中的每个节点
    while (*busy) 
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "while (*busy)\n\n");
    	
        cl = *busy;

        if (cl->buf->tag != tag) 
        {
        	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "if (cl->buf->tag != tag)\n\n");
        	
            *busy = cl->next;
            night_free_chain(p, cl);
            continue;
        }

        if (night_buf_size(cl->buf) != 0) 
        {
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        	dprintf(trace_file_fd, "if (ngx_buf_size(cl->buf) != 0)\n\n");
        	
            break;
        }

        cl->buf->pos = cl->buf->start;
        cl->buf->last = cl->buf->start;

        *busy = cl->next;
        cl->next = *free;
        *free = cl;
    }
}
