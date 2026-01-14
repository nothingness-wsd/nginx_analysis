#ifndef _NIGHT_TIME_H_
#define _NIGHT_TIME_H_


struct night_time_s
{
    time_t	sec;
    time_t  msec;
    int		gmtoff;
};

int
night_time_init();

int
night_time_update();

int64_t    
night_monotonic_time(time_t sec, time_t msec);

void
night_gmtime(time_t t, struct tm *tp);

char *
night_http_time(char *buf, time_t t);

extern volatile night_str_t	night_cached_http_time;

#endif /* _NIGHT_TIME_H_ */
