#ifndef _NIGHT_SIGNAL_H_
#define _NIGHT_SIGNAL_H_

typedef struct night_signal_s night_signal_t;

struct night_signal_s
{
    int 	signo;
    char 	*signame;
    char 	*name;
    void 	(*handler)(int signo, siginfo_t *siginfo, void *ucontext);
};

extern night_signal_t  signals[];

int
night_init_signals();

void
night_signal_handler(int signo, siginfo_t *siginfo, void *ucontext);

#endif /* _NIGHT_SIGNAL_H_ */
