#include "night_core.h"

// 将一个 Unix 时间戳（以秒为单位的 time_t 类型）转换为本地时间（即考虑系统时区的年、月、日、时、分、秒等结构），并将结果填充到 tm 结构体中
void
night_localtime(time_t s, struct tm *tm)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function:\t" "night_localtime\n\n");

	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"localtime_r\n\n");
	
    (void) localtime_r(&s, tm);

	// 标准 C 的 struct tm 中，tm_mon 字段的取值范围是 0–11（0 表示一月），而应用程序期望月份为 1–12
	// 因此，在这里统一将月份“人性化”处理（1 = January）
    tm->tm_mon++;
    
    // 标准 struct tm 的 tm_year 是“自 1900 年起的年数”，例如 2025 年对应 tm_year = 125。
	// 希望直接得到完整的年份（如 2025），所以加上 1900。
    tm->tm_year += 1900;
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd, "function night_localtime:\t" "return\n\n");
}

void
night_timezone_update(void)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_timezone_update\n\n");
	
    time_t      	s;
    struct tm  		*t;
    char        	buf[4];

    s = time(0);

    t = localtime(&s);

    strftime(buf, 4, "%H", t);

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function night_timezone_update:\t" "return\n\n");
}

