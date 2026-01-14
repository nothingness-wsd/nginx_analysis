#include "night_core.h"
#include "night_time.h"

#define NIGHT_TIME_SLOTS (64)

night_time_t 		night_cached_times[NIGHT_TIME_SLOTS];
night_time_t		*night_cached_time;
int					night_cached_time_slot;

volatile uint64_t		night_current_msec;

volatile night_str_t	night_cached_http_time;

static char            	cached_http_time[NIGHT_TIME_SLOTS][sizeof("Mon, 28 Sep 1970 06:00:00 GMT")];

static char  *week[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
static char  *months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                           "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

int
night_time_init()
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_time_init\n\n");
    
    int rc;
    
    night_cached_http_time.len = sizeof("Mon, 28 Sep 1970 06:00:00 GMT") - 1;
    
    night_cached_time_slot = (NIGHT_TIME_SLOTS - 1);
    
    rc = night_time_update();
    if (rc == NIGHT_ERROR)
    {
    	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd,"night_time_update failed\n\n");
    	
    	return NIGHT_ERROR;
    }
    
    night_cached_time = &night_cached_times[0];
    
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"night_cached_time->sec=%ld\n", night_cached_time->sec);
    dprintf(trace_file_fd,"night_cached_time->msec=%ld\n\n", night_cached_time->msec);
    
    return NIGHT_OK;
}    
    
int
night_time_update()
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_time_update\n\n");
    
    struct timeval 	tv;
    night_time_t 	*ct;
    int				rc;
	time_t 			sec;
    time_t 			msec;
    char*			p0;
    struct tm		gmt;
    
    
    rc = gettimeofday(&tv,NULL);
    if (rc == -1)
    {
    	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd,"gettimeofday failed\n\n");
    	
    	return NIGHT_ERROR;
    }
    
	sec = tv.tv_sec;
    msec = tv.tv_usec / 1000;
    
    night_current_msec = night_monotonic_time(sec, msec);
    if(night_current_msec == NIGHT_ERROR)
    {
    	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(error_log_fd,"night_monotonic_time failed\n\n");
    	
    	return NIGHT_ERROR;
    }
    
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"night_current_msec=%ld\n\n", night_current_msec);
    
    ct = &night_cached_times[night_cached_time_slot];
    
	if(ct->sec == sec)
    {
        ct->msec = msec;
        return NIGHT_OK;
    }
    
	if (night_cached_time_slot == (NIGHT_TIME_SLOTS - 1) )
    {
        night_cached_time_slot = 0;
    }
    else
    {
        night_cached_time_slot++;
    }
    
    ct = &night_cached_times[night_cached_time_slot];
    
	ct->sec = sec;
    ct->msec = msec;
    
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"night_cached_time_slot=%d\n\n",night_cached_time_slot);
    
    night_gmtime(sec, &gmt);
    
	p0 = &cached_http_time[night_cached_time_slot][0];

    (void) sprintf(p0, "%s, %02d %s %4d %02d:%02d:%02d GMT",
                       week[gmt.tm_wday], gmt.tm_mday,
                       months[gmt.tm_mon - 1], gmt.tm_year,
                       gmt.tm_hour, gmt.tm_min, gmt.tm_sec);
                       
    night_cached_http_time.data = p0;                   
    
    return NIGHT_OK;
}

int64_t   
night_monotonic_time(time_t sec, time_t msec)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_monotonic_time\n\n");
    
	struct timespec  ts;
	int rc;

	// CLOCK_MONOTONIC 单调递增时间，从系统启动开始计时，不受系统时间调整影响（推荐用于测量时间间隔）
    rc = clock_gettime(CLOCK_MONOTONIC, &ts);
	if(rc == -1)
	{
		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(error_log_fd,"clock_gettime(CLOCK_MONOTONIC, &ts); failed\n\n");
		
		return NIGHT_ERROR;
	}
	
    sec = ts.tv_sec;
    msec = ts.tv_nsec / 1000000;
    
    return (int64_t) (sec * 1000 + msec);
}    
    
void
night_gmtime(time_t t, struct tm *tp)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_gmtime\n\n");

    int   		yday;
    int  		sec, min, hour, mday, mon, year, wday, days, leap;

    /* the calculation is valid for positive time_t only */

    if (t < 0) 
    {
        t = 0;
    }
/*
86400 = 24 * 3600，是一天的秒数。
days：完整的天数（从 1970-01-01 开始）
sec：当天剩余的秒数（0–86399）
*/
    days = t / 86400;
    sec = t % 86400;

    /*
     * no more than 4 year digits supported,
     * truncate to December 31, 9999, 23:59:59
     */

    if (days > 2932896) {
        days = 2932896;
        sec = 86399;
    }

	// 计算星期几（wday）
    // 1970-01-01 是 星期四

    wday = (4 + days) % 7;

	// 分解一天内的时、分、秒
    hour = sec / 3600;
    sec %= 3600;
    min = sec / 60;
    sec %= 60;

/*
将“自 1970 起的天数”转为“自公元前 1 年 3 月 1 日起的天数”
基于 Gauss 的日历算法
为什么从 3 月 1 日开始？
在格里高利历中，将 3 月视为一年的开始可以简化闰年处理（2 月放在年末）
这样，闰日（2 月 29 日）就变成“上一年的最后一天”，避免在年中插入

+ 719527：
这是从 公元前 1 年 3 月 1 日 到 1970 年 3 月 1 日 的总天数

31 + 28 = 59：
1970 年 1 月有 31 天，2 月有 28 天（1970 不是闰年）
*/
    days = days - (31 + 28) + 719527;

/*
初步估算年份

背景知识：
格里高利历每 400 年有：
365 * 400 = 146000 天（平年）
加上闰年：+100（每 4 年一闰）
减去整百年非闰：-4（100, 200, 300, 500... 但 400 是闰）
加回 400 年闰：+1
总计：146000 + 100 - 4 + 1 = 146097 天 / 400 年

+2 是一个经验偏移，用于修正整除带来的误差（因为 3 月 1 日不是年首）
*/

    year = (days + 2) * 400 / (365 * 400 + 100 - 4 + 1);

/*
计算该年内的天数
计算从 该年 3 月 1 日 起经过的天数（即“年内天数”，但以 3 月为起点）。
365 * year：平年天数
+ year / 4：每 4 年加 1 天（闰年）
- year / 100：每 100 年减 1 天（整百年不闰）
+ year / 400：每 400 年加 1 天（400 倍数年仍闰）
这正是格里高利历的闰年规则。
yday 现在表示：从 当年 3 月 1 日 起的天数（0-indexed）。 
*/
    yday = days - (365 * year + year / 4 - year / 100 + year / 400);

/*
校正年份（处理估算误差）
如果 yday < 0，说明年份估算过大，实际日期属于前一年。
例如：估算为 2025 年，但 yday = -5，说明是 2024 年的最后几天。
校正步骤：
计算前一年（year）是否为闰年：
leap = 1 如果是闰年，否则 0
yday = 365 + leap + yday：
把负的 yday 转换为前一年的年末天数
例如 yday = -5 → 365 + leap - 5
year--：年份减 1
现在 year 和 yday 正确对应。 
*/
    if (yday < 0) 
    {
        leap = (year % 4 == 0) && (year % 100 || (year % 400 == 0));
        yday = 365 + leap + yday;
        year--;
    }

/*
由年内天数（yday）估算月份（mon）
这是一个经验公式，用于快速将 yday（从 3 月 1 日起）映射到月份
*/
    mon = (yday + 31) * 10 / 306;

    /*
     	计算当月日期
	这是 Gauss 的月份天数累计公式 的逆运算。
	367 * mon / 12 是一个近似累计天数（考虑大小月）。
	-30 是偏移校正。
	+1 是因为日期从 1 开始（而非 0）
	得到正确的 mday（1–31） 
	*/
    mday = yday - (367 * mon / 12 - 30) + 1;

	// 将“3月起始”的月份转换为标准月份（1–12 → 0–11）
    if (yday >= 306) 
    {
        year++;
        mon -= 10;
    } 
    else 
    {
        mon += 2;
    }

    tp->tm_sec =  	sec;
    tp->tm_min =  	min;
    tp->tm_hour =  	hour;
    tp->tm_mday =  	mday;
    tp->tm_mon =  	mon;
    tp->tm_year =  	year;
    tp->tm_wday =  	wday;
}

char *
night_http_time(char *buf, time_t t)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_http_time\n\n");
    
    struct tm	tm;

    night_gmtime(t, &tm);

	buf += sprintf(buf, "%s, %02d %s %4d %02d:%02d:%02d GMT",
                       week[tm.tm_wday],
                       tm.tm_mday,
                       months[tm.tm_mon - 1],
                       tm.tm_year,
                       tm.tm_hour,
                       tm.tm_min,
                       tm.tm_sec);
                       
   return buf;                    
}
