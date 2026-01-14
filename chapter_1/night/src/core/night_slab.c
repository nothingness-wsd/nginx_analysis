#include "night_core.h"

static size_t  night_slab_max_size;
static size_t  night_slab_exact_size;
static size_t  night_slab_exact_shift;

// 用于在启动时初始化 slab 内存分配器中与内存块大小相关的关键常量。
// 主要用于共享内存中的内存管理
void
night_slab_sizes_init(void)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function:\t" "night_slab_sizes_init\n\n");
	
	size_t 			n;
	
	// 设置 slab 分配器能直接分配的最大内存块大小
	// 超过这个大小的请求，slab 分配器会直接分配完整页面（或多个页面），而不是使用 slab 机制进行精细划分
    night_slab_max_size = night_pagesize / 2;
    
	// slab 分配器将小内存请求分为三类：
	// small：非常小的块（如 8、16、32 字节等），使用位图管理。
	// exact：中等大小的块，其大小恰好使得一页可以被整除，且每个块的头部 metadata 可以内嵌（避免额外开销）。
	// large：接近 ngx_slab_max_size 的块，使用更简单的分配方式。
	// night_slab_exact_size 是“exact”类的块大小。
	// 8 * sizeof(uintptr_t)：这是一页中用于存储“bitmap”的空间大小
	// 位图大小是 8 * sizeof(uintptr_t) 字节 = 8 * 8 * 8 = 512 位（64 位系统）。
	// 因此最多可管理 512 个块。
    night_slab_exact_size = night_pagesize / (8 * sizeof(uintptr_t));
    
    // 计算  log2(night_slab_exact_size)
    for (n = night_slab_exact_size; n >>= 1; night_slab_exact_shift++) 
    {
        /* void */
    }
    
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"night_slab_max_size=%ld\n", night_slab_max_size);
    dprintf(trace_file_fd, "night_slab_exact_size=%ld\n", night_slab_exact_size);
    dprintf(trace_file_fd, "night_slab_exact_shift=%ld\n\n", night_slab_exact_shift);
    dprintf(trace_file_fd, "function night_slab_sizes_init:\t" "return\n\n");
}

void
night_slab_init(night_slab_pool_t *pool)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function:\t" "night_slab_init\n\n");
	
}	
