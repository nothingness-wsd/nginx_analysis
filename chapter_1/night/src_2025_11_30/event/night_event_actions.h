#ifndef _NIGHT_EVENTS_ACTIONS_H_
#define _NIGHT_EVENTS_ACTIONS_H_

struct night_event_actions_s 
{
	int (*init)();
	int (*add_event)(night_event_t *ev, int event, uint32_t flags);
	int (*del_event)(night_event_t *ev, int event, uint32_t flags);
	int (*process_events)(int64_t timer, uint32_t flags);
	int (*del_conn)(night_connection_t *c, uint32_t flags);
	
/*	
	int (*add_conn)(night_connection_t *c);
	void (*done)();
    ngx_int_t  (*enable)(ngx_event_t *ev, ngx_int_t event, ngx_uint_t flags);
    ngx_int_t  (*disable)(ngx_event_t *ev, ngx_int_t event, ngx_uint_t flags);

    ngx_int_t  (*add_conn)(ngx_connection_t *c);

    ngx_int_t  (*notify)(ngx_event_handler_pt handler);

    ngx_int_t  (*init)(ngx_cycle_t *cycle, ngx_msec_t timer);
    
 */   
};

#endif /* _NIGHT_EVENTS_ACTIONS_H_ */
