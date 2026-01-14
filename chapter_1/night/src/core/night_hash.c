#include "night_core.h"

int
night_hash_keys_array_init(night_hash_keys_arrays_t *ha, int type)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "%s\n\n", __func__);
	
    size_t  asize;

    if (type == NIGHT_HASH_SMALL) 
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "小内存配置\n\n");
    	
        asize = 4;
        ha->hsize = 107;

    } else 
    {
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd, "大内存配置\n\n");
    	
        asize = NIGHT_HASH_LARGE_ASIZE;
        ha->hsize = NIGHT_HASH_LARGE_HSIZE;
    }

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "初始化三个动态数组\n\n");
	
    if (night_array_init(&ha->keys, ha->temp_pool, asize, sizeof(night_hash_key_t))
        != NIGHT_OK)
    {
        return NIGHT_ERROR;
    }

    if (night_array_init(&ha->dns_wc_head, ha->temp_pool, asize,
                       sizeof(night_hash_key_t))
        != NIGHT_OK)
    {
        return NIGHT_ERROR;
    }

    if (night_array_init(&ha->dns_wc_tail, ha->temp_pool, asize,
                       sizeof(night_hash_key_t))
        != NIGHT_OK)
    {
        return NIGHT_ERROR;
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "初始化三个动态数组 完成\n\n");
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "分配哈希桶数组\n\n");

    ha->keys_hash = night_pcalloc(ha->temp_pool, sizeof(night_array_t) * ha->hsize);
    if (ha->keys_hash == NULL) 
    {
        return NIGHT_ERROR;
    }

    ha->dns_wc_head_hash = night_pcalloc(ha->temp_pool, sizeof(night_array_t) * ha->hsize);
    if (ha->dns_wc_head_hash == NULL) 
    {
        return NIGHT_ERROR;
    }

    ha->dns_wc_tail_hash = night_pcalloc(ha->temp_pool, sizeof(night_array_t) * ha->hsize);
    if (ha->dns_wc_tail_hash == NULL) 
    {
        return NIGHT_ERROR;
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "分配哈希桶数组 完成\n\n");

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function %s:\t" "return NIGHT_OK\n\n", __func__);
	
    return NIGHT_OK;
}
