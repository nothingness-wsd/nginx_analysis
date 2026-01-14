#include "night_core.h"
#include "night_event_timer.h"
#include "night_rbtree.h"
#include "night_event.h"

night_rbtree_t 		night_event_timer_rbtree;
night_rbtree_node_t night_event_timer_sentinel;

int
night_event_timer_init()
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd, "function:\t" "night_event_timer_init\n\n");
    
    night_rbtree_init(&night_event_timer_rbtree, &night_event_timer_sentinel, night_rbtree_insert_timer_value);

    return NIGHT_OK;
}


/*
用于向 nginx 的 事件定时器红黑树（rbtree） 中添加一个带有超时时间的事件。
*/
void
night_event_add_timer(night_event_t *ev, int64_t timer)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"function:\t"  "night_event_add_timer\n\n");
    
    int64_t key;
    int64_t diff;
    
    // 计算绝对超时时间
	// 计算绝对超时时间
	// ngx_current_msec：nginx 全局变量，记录当前毫秒时间（由事件循环定期更新）。
	// 定时器在红黑树中按 绝对时间（key） 排序，最小 key 在最左，便于快速找到最早超时的事件。
    key = night_current_msec + timer;
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd, "key = night_current_msec + timer;\n");
    dprintf(trace_file_fd, "%ld = %ld + %ld;\n\n", key, night_current_msec, timer);
    
    // 检查该事件是否已设置定时器
    if (ev->timer_set) 
    {
		dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
		dprintf(trace_file_fd,"已设置定时器\n\n");
		
    	diff = (int64_t) (key - ev->timer.key);
    	
		if (night_abs(diff) < NIGHT_TIMER_LAZY_DELAY) 
		{
            return;
        }
        
        night_event_del_timer(ev);
    }
    
    // 设置新的定时器 key
    ev->timer.key = key;
    
    // 将事件的定时器节点插入全局定时器红黑树
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
	dprintf(trace_file_fd,"night_rbtree_insert\n\n");
	
    night_rbtree_insert(&night_event_timer_rbtree, &ev->timer);

	// 标记该事件已加入定时器红黑树，防止重复插入
    ev->timer_set = 1;
}

void
night_event_del_timer(night_event_t *ev)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"function:\t"  "night_event_del_timer\n\n");
    
    night_rbtree_delete(&night_event_timer_rbtree, &ev->timer);

	ev->timer.key = 0;
    ev->timer.left = NULL;
    ev->timer.right = NULL;
    ev->timer.parent = NULL;

    ev->timer_set = 0;
}

int64_t
night_event_find_timer(void)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"function:\t"  "night_event_find_timer\n\n");
    
    int64_t      			timer;
    night_rbtree_node_t  	*node, *root, *sentinel;

    if (night_event_timer_rbtree.root == &night_event_timer_sentinel) 
    {
        return NIGHT_TIMER_INFINITE;
    }

    root = night_event_timer_rbtree.root;
    sentinel = night_event_timer_rbtree.sentinel;

    node = night_rbtree_min(root, sentinel);

    timer = node->key - night_current_msec;

    return timer > 0 ? timer : 0;
}

