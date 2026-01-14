#ifndef _NIGHT_OS_H_
#define _NIGHT_OS_H_

#define NIGHT_IOVS_PREALLOCATE  64

typedef struct night_iovec_s night_iovec_t;

struct night_iovec_s
{
    struct iovec  		*iovs;
    size_t     			count;
    size_t         		size;
    size_t     			nalloc;
};

int
night_os_init();

int
night_init_setproctitle();

#endif /* _NIGHT_OS_H_ */
