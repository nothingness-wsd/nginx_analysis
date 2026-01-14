#include "night_core.h"

#define NIGHT_TIME_SLOTS (64)

static 		night_time_t		cached_time[NIGHT_TIME_SLOTS];
static 		char            	cached_http_time[NIGHT_TIME_SLOTS][sizeof("Mon, 28 Sep 1970 06:00:00 GMT")];
                                    
volatile 	night_time_t		*night_cached_time;
volatile 	night_str_t			night_cached_http_time;

static 		night_atomic_t      night_time_lock;

volatile 	uint64_t			night_current_msec;

static 		uint32_t        	slot;

static 		int64_t         	cached_gmtoff;

static char* week[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
static char* months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                           "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };


void
night_time_init()
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_time_init\n\n");
    
    // 时间格式长度设置
    night_cached_http_time.len = sizeof("Mon, 28 Sep 1970 06:00:00 GMT") - 1;
    
	// 设置全局时间缓存指针，指向缓存数组的第一个元素
    night_cached_time = &cached_time[0];

	// 首次更新系统时间
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"首次更新系统时间\n" "night_time_update()\n\n");
	
    night_time_update();
    
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"首次更新系统时间完成\n" "night_time_update() completed\n\n");
	
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function night_time_init:\t" "return\n\n");
}

void
night_time_update()
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_time_update\n\n");
    
    int						rc;
    struct timeval   		tv;
	time_t           		sec;
    int64_t       			msec;
    night_time_t      		*tp;
    struct tm         		tm, gmt;
    char*					p0;
    
	// 非阻塞锁尝试，如果锁已被占用则立即返回 0
	// 如果获取锁失败（返回 0），则直接返回，不更新时间
	// 线程安全：确保在多 worker 进程或多线程环境下，只有一个进程/线程能同时更新时间缓存
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"night_time_lock=%d\n\n", night_time_lock);
	
	rc = night_trylock(&night_time_lock);
    if (!rc) 
    {
    	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd,"获取锁失败\n" "function night_time_update:\treturn\n\n");
    	
        return;
    }
    
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"night_time_lock=%d\n\n", night_time_lock);
	
	// 获取当前精确时间
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"获取当前精确时间\n" "night_gettimeofday(&tv)\n\n");
	
    night_gettimeofday(&tv);
    
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"night_gettimeofday(&tv) completed\n\n");
    
	//从 timeval 结构中提取秒和毫秒
	//tv.tv_sec：直接获取秒数
    sec = tv.tv_sec;
    
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"sec=%ld\n\n", sec);
    
	//tv.tv_usec / 1000：将微秒转换为毫秒（1秒 = 1,000,000微秒，1秒 = 1,000毫秒）    
    msec = tv.tv_usec / 1000;
    
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"msec=%ld\n\n", msec);
    
	// 更新全局单调时间计数器
    // 计算从 Nginx 启动开始的毫秒数
	// 通常基于系统启动时间或进程启动时间，不受系统时间调整影响
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"更新全局单调时间计数器\n" "night_monotonic_time(sec, msec)\n\n");
	
    night_current_msec = night_monotonic_time();
    
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"更新全局单调时间计数器完成\n" 
		"night_monotonic_time(sec, msec) completed\n" 
		"night_current_msec=%ld\n\n", night_current_msec);
	
	// 获取当前时间槽的指针
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"获取当前时间槽的指针\n" "slot=%d\n\n", slot);
	
    tp = &cached_time[slot];
    
	// 检查是否在同一秒内
	// 如果当前槽位的时间秒数与新时间相同，在同一秒内只需要更新毫秒部分
	// 更新后立即释放锁并返回
    if (tp->sec == sec) 
    {
        tp->msec = msec;
        night_unlock(&night_time_lock);
        
        dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(trace_file_fd,"night_time_lock=%d\n"
        	"function night_time_update:\treturn\n\n", night_time_lock);
        
        return;
    }
    
	// 循环更新时间槽
	// 当到达最后一个槽位时，回到第一个槽位（0）
	// 否则移动到下一个槽位
	// 实现类似环形缓冲区的机制
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"循环更新时间槽\n" "slot=%d\n\n", slot);
	
    if (slot == NIGHT_TIME_SLOTS - 1) 
    {
        slot = 0;
    } 
    else 
    {
        slot++;
    }
    
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"更新后\n" "slot=%d\n\n", slot);
	
	// 指向新的时间槽
    tp = &cached_time[slot];

	// 存储新的时间值到当前槽位
    tp->sec = sec;
    tp->msec = msec;
    
	// 将秒数转换为 GMT 时间
	// 封装 gmtime() 系统调用，将 Unix 时间戳转换为 GMT 时间结构
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"将时间戳转换为结构化的日历时间\n" "night_gmtime\n\n");
	
    night_gmtime(sec, &gmt);
    
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"将时间戳转换为结构化的日历时间完成\n" "night_gmtime completed\n\n");
	
	// 获取 HTTP 时间字符串缓冲区的指针
    p0 = &cached_http_time[slot][0];
    
	// 格式化 HTTP 标准时间字符串
    sprintf(p0, "%s, %02d %s %4d %02d:%02d:%02d GMT",
                       week[gmt.tm_wday], gmt.tm_mday,
                       months[gmt.tm_mon - 1], gmt.tm_year,
                       gmt.tm_hour, gmt.tm_min, gmt.tm_sec);

	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"格式化 HTTP 标准时间字符串\n" "p0=%s\n\n", p0);
	
	// 时区处理
	// 获取本地时间
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"获取本地时间\n" "night_localtime(sec, &tm)\n\n");
	
	night_localtime(sec, &tm);
	
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"获取本地时间完成\n" "night_localtime(sec, &tm) completed\n\n");
    
	// 从本地时间结构中提取时区偏移
    // tm_gmtoff 通常以秒为单位，转换为分钟
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"时区偏移\n");
    
    cached_gmtoff = tm.tm_gmtoff / 60;
    tp->gmtoff = cached_gmtoff;
    
    dprintf(trace_file_fd,"cached_gmtoff=%ld\n\n", cached_gmtoff);
    
	// 插入内存屏障
	// 确保所有之前的内存写操作对其他 CPU 核心可见
	// 阻止编译器和 CPU 对内存操作进行重排序
    night_memory_barrier();
    
	// 更新全局时间指针
    night_cached_time = tp;
    
     // 更新全局时间字符串指针
    night_cached_http_time.data = p0;
    
	// 释放时间更新锁
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "释放时间更新锁\n" "night_unlock(&night_time_lock)\n\n");
	
    night_unlock(&night_time_lock);
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "已释放时间更新锁\n" "night_time_lock=%d\n\n", night_time_lock);
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "function night_time_update:\t" "return\n\n");
	
}

static uint64_t
night_monotonic_time()
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function:\t" "night_monotonic_time\n\n");
	
    struct timespec  	ts;
    time_t 				sec; 
    int64_t 			msec;

	// POSIX标准的系统调用，用于获取高精度、单调递增的时间
	// CLOCK_MONOTONIC：第一个参数，指定要使用的时钟类型
	// &ts：第二个参数，指向timespec结构体的指针，用于存储获取到的时间值
 	// CLOCK_MONOTONIC：标准 POSIX 单调时钟，通过系统调用获取，精度高（通常纳秒级）
 	// 单调递增：时间值只会向前增长，永远不会回退
 	// 从系统启动开始计时：通常从系统启动时刻（boot time）开始计数，不包含系统休眠时间
    clock_gettime(CLOCK_MONOTONIC, &ts);
	
	// 将 timespec 中的时间转换为秒和毫秒
    sec = ts.tv_sec;
    msec = ts.tv_nsec / 1000000;

	// 计算并返回毫秒值
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function night_monotonic_time:\t" "retun\n\n");
	
    return (uint64_t) sec * 1000 + msec;
}


void
night_gmtime(time_t t, struct tm *tp)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_gmtime\n\n");

    uint64_t   		yday;
    uint64_t  		sec, min, hour, mday, mon, year, wday, days, leap;

	// 防止无效输入，将负值强制设为0
    if (t < 0) 
    {
        t = 0;
    }

	// 86400：一天的总秒数（24×60×60）
	// days：从1970-01-01开始经过的完整天数
	// sec：当前天已过去的秒数（0-86399）
    
    days = t / 86400;
    sec = t % 86400;

	// 2932896天：从1970-01-01到9999-12-31的天数
	// 86399秒：一天的最后一秒（23:59:59）
	// 限制时间范围在公元1年到9999年之间，符合常见日期表示需求
	// 防止整数溢出和无效日期，确保结果在合理范围内
    if (days > 2932896) 
    {
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
    
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function night_gmtime:\t" "return\n\n");
}

   
