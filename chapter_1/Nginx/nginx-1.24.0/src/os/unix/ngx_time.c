
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


/*
 * FreeBSD does not test /etc/localtime change, however, we can workaround it
 * by calling tzset() with TZ and then without TZ to update timezone.
 * The trick should work since FreeBSD 2.1.0.
 *
 * Linux does not test /etc/localtime change in localtime(),
 * but may stat("/etc/localtime") several times in every strftime(),
 * therefore we use it to update timezone.
 *
 * Solaris does not test /etc/TIMEZONE change too and no workaround available.
 */

void
ngx_timezone_update(void)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function:\t" "ngx_timezone_update\n\n");
	
#if (NGX_FREEBSD)

    if (getenv("TZ")) {
        return;
    }

    putenv("TZ=UTC");

    tzset();

    unsetenv("TZ");

    tzset();

#elif (NGX_LINUX)

	// s：存储当前时间（自 Unix 纪元以来的秒数）。
	// t：指向 struct tm 的指针，用于存储分解后的时间。
	// buf[4]：足够存储 "HH\0"（两位小时 + 终止符）的缓冲区

    time_t      s;
    struct tm  *t;
    char        buf[4];

	// 调用 time(NULL) 获取当前时间戳
    s = time(0);

	// 调用 localtime() 将时间戳转换为本地时间结构
    t = localtime(&s);

	// 格式化时间（但结果未使用！）
	// 使用 strftime() 将小时格式化到 buf 中。
	// 但是：buf 是局部变量，函数结束后就销毁，且没有其他用途。
	// 这段代码实际上没有任何副作用
    strftime(buf, 4, "%H", t);
    
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function ngx_timezone_update:\t" "return\n\n");

#endif
}


// 将一个 Unix 时间戳（以秒为单位的 time_t 类型）转换为本地时间（即考虑系统时区的年、月、日、时、分、秒等结构），并将结果填充到 ngx_tm_t 结构体中
void
ngx_localtime(time_t s, ngx_tm_t *tm)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function:\t" "ngx_localtime\n\n");
	
#if (NGX_HAVE_LOCALTIME_R)

	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"localtime_r\n\n");
	
    (void) localtime_r(&s, tm);

#else
    ngx_tm_t  *t;

    t = localtime(&s);
    *tm = *t;

#endif

	// 标准 C 的 struct tm 中，tm_mon 字段的取值范围是 0–11（0 表示一月），而许多应用程序（包括 Nginx 内部逻辑）期望月份为 1–12。
	// 因此，Nginx 在这里统一将月份“人性化”处理（1 = January）。
    tm->ngx_tm_mon++;
    
    // 标准 struct tm 的 tm_year 是“自 1900 年起的年数”，例如 2025 年对应 tm_year = 125。
	// Nginx 希望直接得到完整的年份（如 2025），所以加上 1900。
    tm->ngx_tm_year += 1900;
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "function ngx_localtime:\t" "return\n\n");
}


void
ngx_libc_localtime(time_t s, struct tm *tm)
{
#if (NGX_HAVE_LOCALTIME_R)
    (void) localtime_r(&s, tm);

#else
    struct tm  *t;

    t = localtime(&s);
    *tm = *t;

#endif
}


void
ngx_libc_gmtime(time_t s, struct tm *tm)
{
#if (NGX_HAVE_LOCALTIME_R)
    (void) gmtime_r(&s, tm);

#else
    struct tm  *t;

    t = gmtime(&s);
    *tm = *t;

#endif
}
