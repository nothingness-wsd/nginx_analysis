# 头文件目录
INCLUDE = -I./src/core -I./src/event -I./src/event/modules -I./src/http -I./src/http/modules -I./src/os/unix

# 声明伪目标
.PHONY: all build clean

# 默认目标
all: build

# 编译目标
build: \
	objs/src/core/night.o \
	objs/src/core/night_times.o \
	objs/src/os/unix/night_time.o \
	objs/src/core/night_palloc.o \
	objs/src/os/unix/night_alloc.o \
	objs/src/core/night_string.o \
	objs/src/os/unix/night_posix_init.o \
	objs/src/os/unix/night_linux_init.o \
	objs/src/os/unix/night_setproctitle.o \
	objs/src/core/night_slab.o \
	objs/src/core/night_modules.o \
	objs/src/core/night_cycle.o \
	objs/src/core/night_array.o \
	objs/src/core/night_list.o \
	objs/src/core/night_conf.o \
	objs/src/os/unix/night_files.o \
	objs/src/core/night_core_module.o \
	objs/src/core/night_conf_file.o \
	objs/src/core/night_file.o \
	objs/src/event/night_events_module.o \
	objs/src/event/night_event_core_module.o \
	objs/src/event/modules/night_epoll_module.o \
	objs/src/http/night_http_module.o \
	objs/src/http/night_http_core_module.o \
	objs/src/http/night_http_variables.o \
	objs/src/core/night_hash.o
	
	
	gcc -o night \
	objs/src/core/night.o \
	objs/src/core/night_times.o \
	objs/src/os/unix/night_time.o \
	objs/src/core/night_palloc.o \
	objs/src/os/unix/night_alloc.o \
	objs/src/core/night_string.o \
	objs/src/os/unix/night_posix_init.o \
	objs/src/os/unix/night_linux_init.o \
	objs/src/os/unix/night_setproctitle.o \
	objs/src/core/night_slab.o \
	objs/src/core/night_modules.o \
	objs/src/core/night_cycle.o \
	objs/src/core/night_array.o \
	objs/src/core/night_list.o \
	objs/src/core/night_conf.o \
	objs/src/os/unix/night_files.o \
	objs/src/core/night_core_module.o \
	objs/src/core/night_conf_file.o \
	objs/src/core/night_file.o \
	objs/src/event/night_events_module.o \
	objs/src/event/night_event_core_module.o \
	objs/src/event/modules/night_epoll_module.o \
	objs/src/http/night_http_module.o \
	objs/src/http/night_http_core_module.o \
	objs/src/http/night_http_variables.o \
	objs/src/core/night_hash.o
	

objs/src/core/night.o: \
	src/core/night.c \
	src/core/night.h \
	src/core/night_core.h
	
	gcc -c $(INCLUDE) \
	-o objs/src/core/night.o \
	src/core/night.c
	
objs/src/core/night_times.o: \
	src/core/night_times.c \
	src/core/night_times.h \
	src/core/night_core.h
	
	gcc -c $(INCLUDE) \
	-o objs/src/core/night_times.o \
	src/core/night_times.c
	
objs/src/os/unix/night_time.o: \
	src/os/unix/night_time.c \
	src/os/unix/night_time.h \
	src/core/night_core.h
	
	gcc -c $(INCLUDE) \
	-o objs/src/os/unix/night_time.o \
	src/os/unix/night_time.c

objs/src/core/night_palloc.o: \
	src/core/night_palloc.c \
	src/core/night_palloc.h \
	src/core/night_core.h
	
	gcc -c $(INCLUDE) \
	-o objs/src/core/night_palloc.o \
	src/core/night_palloc.c
	
objs/src/os/unix/night_alloc.o: \
	src/os/unix/night_alloc.c \
	src/os/unix/night_alloc.h \
	src/core/night_core.h
	
	gcc -c $(INCLUDE) \
	-o objs/src/os/unix/night_alloc.o \
	src/os/unix/night_alloc.c
	
objs/src/core/night_string.o: \
	src/core/night_string.c \
	src/core/night_string.h \
	src/core/night_core.h
	
	gcc -c $(INCLUDE) \
	-o objs/src/core/night_string.o \
	src/core/night_string.c
	
objs/src/os/unix/night_posix_init.o: \
	src/os/unix/night_posix_init.c \
	src/os/unix/night_posix_init.h \
	src/core/night_core.h
	
	gcc -c $(INCLUDE) \
	-o objs/src/os/unix/night_posix_init.o \
	src/os/unix/night_posix_init.c
	
objs/src/os/unix/night_linux_init.o: \
	src/os/unix/night_linux_init.c \
	src/os/unix/night_linux_init.h \
	src/core/night_core.h
	
	gcc -c $(INCLUDE) \
	-o objs/src/os/unix/night_linux_init.o \
	src/os/unix/night_linux_init.c
	
objs/src/os/unix/night_setproctitle.o: \
	src/os/unix/night_setproctitle.c \
	src/os/unix/night_setproctitle.h \
	src/core/night_core.h
	
	gcc -c $(INCLUDE) \
	-o objs/src/os/unix/night_setproctitle.o \
	src/os/unix/night_setproctitle.c
	
objs/src/core/night_slab.o: \
	src/core/night_slab.c \
	src/core/night_slab.h \
	src/core/night_core.h
	
	gcc -c $(INCLUDE) \
	-o objs/src/core/night_slab.o \
	src/core/night_slab.c
	
objs/src/core/night_modules.o: \
	src/core/night_modules.c \
	src/core/night_modules.h \
	src/core/night_core.h
	
	gcc -c $(INCLUDE) \
	-o objs/src/core/night_modules.o \
	src/core/night_modules.c
	
objs/src/core/night_cycle.o: \
	src/core/night_cycle.c \
	src/core/night_cycle.h \
	src/core/night_core.h
	
	gcc -c $(INCLUDE) \
	-o objs/src/core/night_cycle.o \
	src/core/night_cycle.c
	
objs/src/core/night_array.o: \
	src/core/night_array.c \
	src/core/night_array.h \
	src/core/night_core.h
	
	gcc -c $(INCLUDE) \
	-o objs/src/core/night_array.o \
	src/core/night_array.c
	
objs/src/core/night_list.o: \
	src/core/night_list.c \
	src/core/night_list.h \
	src/core/night_core.h
	
	gcc -c $(INCLUDE) \
	-o objs/src/core/night_list.o \
	src/core/night_list.c
	
objs/src/core/night_conf.o: \
	src/core/night_conf.c \
	src/core/night_conf.h \
	src/core/night_core.h
	
	gcc -c $(INCLUDE) \
	-o objs/src/core/night_conf.o \
	src/core/night_conf.c
	
objs/src/os/unix/night_files.o: \
	src/os/unix/night_files.c \
	src/os/unix/night_files.h \
	src/core/night_core.h
	
	gcc -c $(INCLUDE) \
	-o objs/src/os/unix/night_files.o \
	src/os/unix/night_files.c
	
objs/src/core/night_core_module.o: \
	src/core/night_core_module.c \
	src/core/night_core_module.h \
	src/core/night_core.h
	
	gcc -c $(INCLUDE) \
	-o objs/src/core/night_core_module.o \
	src/core/night_core_module.c
	
objs/src/core/night_conf_file.o: \
	src/core/night_conf_file.c \
	src/core/night_conf_file.h \
	src/core/night_core.h
	
	gcc -c $(INCLUDE) \
	-o objs/src/core/night_conf_file.o \
	src/core/night_conf_file.c
	
objs/src/core/night_file.o: \
	src/core/night_file.c \
	src/core/night_file.h \
	src/core/night_core.h
	
	gcc -c $(INCLUDE) \
	-o objs/src/core/night_file.o \
	src/core/night_file.c

objs/src/event/night_events_module.o: \
	src/event/night_events_module.c \
	src/event/night_events_module.h \
	src/core/night_core.h
	
	gcc -c $(INCLUDE) \
	-o objs/src/event/night_events_module.o \
	src/event/night_events_module.c
	
objs/src/event/night_event_core_module.o: \
	src/event/night_event_core_module.c \
	src/event/night_event_core_module.h \
	src/core/night_core.h
	
	gcc -c $(INCLUDE) \
	-o objs/src/event/night_event_core_module.o \
	src/event/night_event_core_module.c
	
objs/src/event/modules/night_epoll_module.o: \
	src/event/modules/night_epoll_module.c \
	src/core/night_core.h
	
	gcc -c $(INCLUDE) \
	-o objs/src/event/modules/night_epoll_module.o \
	src/event/modules/night_epoll_module.c
	
objs/src/http/night_http_module.o: \
	src/http/night_http_module.c \
	src/core/night_core.h
	
	gcc -c $(INCLUDE) \
	-o objs/src/http/night_http_module.o \
	src/http/night_http_module.c

objs/src/http/night_http_core_module.o: \
	src/http/night_http_core_module.c \
	src/core/night_core.h
	
	gcc -c $(INCLUDE) \
	-o objs/src/http/night_http_core_module.o \
	src/http/night_http_core_module.c
		
objs/src/http/night_http_variables.o: \
	src/http/night_http_variables.c \
	src/core/night_core.h
	
	gcc -c $(INCLUDE) \
	-o objs/src/http/night_http_variables.o \
	src/http/night_http_variables.c
	
objs/src/core/night_hash.o: \
	src/core/night_hash.c \
	src/core/night_core.h
	
	gcc -c $(INCLUDE) \
	-o objs/src/core/night_hash.o \
	src/core/night_hash.c
		

	
# 清理目标
clean:
	rm -f night && \
	find objs/src/ -type f -delete
