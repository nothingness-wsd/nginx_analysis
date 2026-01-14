#ifndef _NIGHT_MODULES_H_
#define _NIGHT_MODULES_H_

#define NIGHT_MODULE_UNSET_INDEX	(-1)

extern
night_module_t  night_core_module;

extern
night_module_t	night_events_module;

extern
night_module_t	night_event_core_module;

extern
night_module_t	night_epoll_module;

extern
night_module_t	night_http_module;

extern
night_module_t	night_http_core_module;

int
night_preinit_modules(void);

int
night_cycle_modules(night_cycle_t *cycle);

int
night_count_modules(night_cycle_t *cycle, uint64_t type);

#endif /* _NIGHT_MODULES_H_ */
