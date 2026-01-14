#ifndef _NIGHT_TIME_H_
#define _NIGHT_TIME_H_

#define night_gettimeofday(tp)  (void) gettimeofday(tp, NULL);

void
night_localtime(time_t s, struct tm *tm);

void
night_timezone_update(void);

#endif /* _NIGHT_TIME_H_ */
