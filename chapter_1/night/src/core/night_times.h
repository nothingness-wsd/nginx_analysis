#ifndef _NIGHT_TIMES_H_
#define _NIGHT_TIMES_H_

typedef struct night_time_s night_time_t;

struct night_time_s
{
    time_t	sec;
    time_t  msec;
    int		gmtoff;
};

void
night_time_init();

void
night_time_update();

static uint64_t
night_monotonic_time();

void
night_gmtime(time_t t, struct tm *tp);

#endif /* _NIGHT_TIMES_H_ */
