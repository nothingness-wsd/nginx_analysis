
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_EVENT_TIMER_H_INCLUDED_
#define _NGX_EVENT_TIMER_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>


#define NGX_TIMER_INFINITE  (ngx_msec_t) -1

#define NGX_TIMER_LAZY_DELAY  300


ngx_int_t ngx_event_timer_init(ngx_log_t *log);
ngx_msec_t ngx_event_find_timer(void);
void ngx_event_expire_timers(void);
ngx_int_t ngx_event_no_timers_left(void);


extern ngx_rbtree_t  ngx_event_timer_rbtree;


static ngx_inline void
ngx_event_del_timer(ngx_event_t *ev)
{
    ngx_log_debug2(NGX_LOG_DEBUG_EVENT, ev->log, 0,
                   "event timer del: %d: %M",
                    ngx_event_ident(ev->data), ev->timer.key);

    ngx_rbtree_delete(&ngx_event_timer_rbtree, &ev->timer);

#if (NGX_DEBUG)
    ev->timer.left = NULL;
    ev->timer.right = NULL;
    ev->timer.parent = NULL;
#endif

    ev->timer_set = 0;
}

/*
用于向 nginx 的 事件定时器红黑树（rbtree） 中添加一个带有超时时间的事件。
*/
static ngx_inline void
ngx_event_add_timer(ngx_event_t *ev, ngx_msec_t timer)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
	dprintf(trace_file_fd,"function:\t" "ngx_event_add_timer\n\n");
	
    ngx_msec_t      key;
    ngx_msec_int_t  diff;

	// 计算绝对超时时间
	// ngx_current_msec：nginx 全局变量，记录当前毫秒时间（由事件循环定期更新）。
	// 定时器在红黑树中按 绝对时间（key） 排序，最小 key 在最左，便于快速找到最早超时的事件。
    key = ngx_current_msec + timer;
    
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"key = ngx_current_msec + timer;\n");
    dprintf(trace_file_fd,"%ld = %ld + %ld;\n\n", key, ngx_current_msec, timer);

	// 检查该事件是否已设置定时器
    if (ev->timer_set) {
    
		dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
		dprintf(trace_file_fd,"已设置定时器\n\n");
		
        /*
         * Use a previous timer value if difference between it and a new
         * value is less than NGX_TIMER_LAZY_DELAY milliseconds: this allows
         * to minimize the rbtree operations for fast connections.
         */

        diff = (ngx_msec_int_t) (key - ev->timer.key);

        if (ngx_abs(diff) < NGX_TIMER_LAZY_DELAY) {
            ngx_log_debug3(NGX_LOG_DEBUG_EVENT, ev->log, 0,
                           "event timer: %d, old: %M, new: %M",
                            ngx_event_ident(ev->data), ev->timer.key, key);
            return;
        }

        ngx_del_timer(ev);
    }

	// 设置新的定时器 key
    ev->timer.key = key;

    ngx_log_debug3(NGX_LOG_DEBUG_EVENT, ev->log, 0,
                   "event timer add: %d: %M:%M",
                    ngx_event_ident(ev->data), timer, ev->timer.key);

	// 将事件的定时器节点插入全局定时器红黑树
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
	dprintf(trace_file_fd,"ngx_rbtree_insert\n\n");
	
    ngx_rbtree_insert(&ngx_event_timer_rbtree, &ev->timer);

	// 标记该事件已加入定时器红黑树，防止重复插入
    ev->timer_set = 1;
}


#endif /* _NGX_EVENT_TIMER_H_INCLUDED_ */
