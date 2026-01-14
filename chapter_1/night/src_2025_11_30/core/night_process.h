#ifndef _NIGHT_PROCESS_H_
#define _NIGHT_PROCESS_H_

#define NIGHT_MAX_PROCESSES (64)

#define NIGHT_INVALID_PID (-1)


typedef void 	(*night_spawn_proc_pt) (void* data);

struct night_process_s
{
    pid_t pid;
    int channel[2];
    int exited;
    int exiting;
    night_spawn_proc_pt proc;
    void* data;
    char* name;
    int detached;
    int status;
};

extern night_process_t 			night_processes[];
extern int						night_process_slot;

pid_t
night_spawn_process(night_spawn_proc_pt proc, void *data, char *name);

void
night_process_get_status(void);

void
night_signal_worker_processes(int signo);

int
night_reap_children();

#endif /* _NIGHT_PROCESS_H_ */

