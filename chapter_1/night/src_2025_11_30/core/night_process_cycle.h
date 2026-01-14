#ifndef _NIGHT_PROCESS_CYCLE_H_
#define _NIGHT_PROCESS_CYCLE_H_  

void
night_master_process_cycle();

void
night_start_worker_processes(int n);

void
night_worker_process_cycle(void *data);

void
night_master_process_exit();

void
night_worker_process_init(int worker);

void 
night_worker_process_exit();

void
night_set_shutdown_timer();

void
night_shutdown_timer_handler(night_event_t *ev);

#endif /* _NIGHT_PROCESS_CYCLE_H_ */

