#include "night_core.h"

// 按照指定的对齐字节数分配一块内存
void*
night_memalign(size_t alignment, size_t size)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function:\t" "night_memalign\n\n");
	
    void  *p;
    int    err;

	// 调用标准 POSIX 函数 posix_memalign 来分配对齐内存。
	// 第一个参数 &p：传入指针的地址，函数会将分配的内存地址写入 p。
	// 第二个参数 alignment：对齐边界（如 16、32、64 字节等）。
	// 第三个参数 size：请求分配的字节数。
	// 返回值：
	// 成功时返回 0；
	// 失败时返回一个非零的错误码（如 EINVAL、ENOMEM 等）。
    err = posix_memalign(&p, alignment, size);

    if (err) 
    {
    	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd,"posix_memalign(%ld, %ld) failed\n\n", alignment, size);
    	
        p = NULL;
    }

	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function night_memalign:\t" "return\n\n");
	
    return p;
}
