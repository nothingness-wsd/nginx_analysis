#include "night_core.h"

ssize_t
night_read_file(night_file_t *file, char *buf, size_t size, off_t offset)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function:\t" "%s\n\n", __func__);

    ssize_t  n;
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "读取文件数据\n" "pread(%d, %p, %ld, %ld)\n\n", file->fd, buf, size, offset);
	
    n = pread(file->fd, buf, size, offset);

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "读取文件数据 完成\n" "n=%ld\n\n", n);
	
	if (n == -1) 
	{                 
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "pread() %s failed\n", file->name.data);
		dprintf(trace_file_fd,"function %s:\t" "return NIGHT_ERROR\n\n", __func__);
		              
        return NIGHT_ERROR;
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "更新文件偏移量\n");
	
	file->offset += n;
	
	dprintf(trace_file_fd, "file->offset=%ld\n\n", file->offset);

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function %s:\t" "return %ld\n\n", __func__, n);
	
    return n;
}
