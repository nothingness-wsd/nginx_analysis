
#include "night_core.h"
#include "night_hash.h"
#include "night_string.h"
#include "night_pool.h"

// 在哈希表中查找指定键对应的值
// night_hash_t *hash：指向一个已初始化的哈希表结构
// uint64_t key:由调用者预先计算好的哈希键值
// char *name 要查找的键名（字符串）
// 键名 name 的长度（字节数）
void*
night_hash_find(night_hash_t *hash, uint64_t key, char *name, size_t len)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
	dprintf(trace_file_fd, "function:\t" "night_hash_find\n\n");
	
	size_t       		i;
    night_hash_elt_t  	*elt;
    night_hash_elt_t	**buckets;
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd, "name=");
    write(trace_file_fd, name, len);
    dprintf(trace_file_fd, "\n\n");
    
	// 查看 
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd, "hash table:\n\n");
    
    buckets = hash->buckets;
    for (i = 0; i < hash->size; i++)
    {
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    	dprintf(trace_file_fd, "i=%ld\n", i);
    	
    	if (buckets[i] == NULL)
    	{
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    		dprintf(trace_file_fd, "continue\n\n");
    	
    		continue;
    	}
    	
    	elt = buckets[i];
        while(elt->value)
        {
			write(trace_file_fd, elt->name, elt->len);
        	dprintf(trace_file_fd, "\n\n");
        	
        	elt = (night_hash_elt_t*) night_align_ptr(&elt->name[0] + elt->len, sizeof(void *));
        } 
    }
    dprintf(trace_file_fd, "\n");

    // hash->buckets 是一个指针数组，每个元素指向一个桶
    // 算哈希值对桶数量取模，得到桶索引
	elt = hash->buckets[key % hash->size];
	
	if (elt == NULL) 
	{
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
		dprintf(trace_file_fd, "if (elt == NULL)\nreturn NULL;\n\n" );
		
        return NULL;
    }
    
    // 遍历桶内链表
    while (elt->value)
    {	
    	// 长度快速比较
    	if (len != (size_t) elt->len )
    	{
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    		dprintf(trace_file_fd, "if (len != (size_t) elt->len )\n%ld != %d\ngoto next;\n", len, elt->len);
    	
    		goto next;
    	}
    	
    	// 逐字节比较键名
		for (i = 0; i < len; i++) 
		{
            if (name[i] != elt->name[i]) 
            {
            	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
            	dprintf(trace_file_fd, "if (name[i] != elt->name[i])\ngoto next;\n\n");
            	
                goto next;
            }
        }
        
        // 匹配成功，返回值
        dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        dprintf(trace_file_fd, "return elt->value;\n\n");
        
        return elt->value;
        
    next:
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    	dprintf(trace_file_fd, "next:\n\n");
    	
    	// 用于跳过当前元素，准备处理下一个元素
		elt = (night_hash_elt_t*) night_align_ptr(&elt->name[0] + elt->len, sizeof(void *));
		
        continue;    
    }
    
    // 当 while 循环因 elt->value == NULL 退出时，说明遍历完整个桶仍未找到匹配项，返回 NULL
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd, "return NULL;\n\n");
    
    return NULL;
}	

uint64_t
night_hash_key_lc(char *data, size_t len)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
	dprintf(trace_file_fd, "function:\t" "night_hash_key_lc\n\n");
	
    uint64_t  i, key;

    key = 0;

    for (i = 0; i < len; i++) 
    {
        key = night_hash(key, night_tolower(data[i]));
    }

    return key;
}

int
night_hash_init(night_hash_init_t *hinit, night_hash_key_t *names, size_t nelts)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
	dprintf(trace_file_fd, "function:\t" "night_hash_init\n\n");
	
	size_t					n;
	uint16_t         		*test;
	night_hash_elt_t		**buckets;
	size_t					start;
	size_t					size;
	size_t					i;
	size_t           		len;
	size_t					bucket_size;
	uint64_t 				key;
	char					*elts;
	night_hash_elt_t		*elt;
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
	dprintf(trace_file_fd, "nelts=%ld\n\n", nelts);
	
	// 检查 max_size 是否为 0
	if (hinit->max_size == 0) 
	{
		dprintf(error_log_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
		dprintf(error_log_fd, "could not build %s, you should increase %s_max_size: %ld\n\n", hinit->name, hinit->name, hinit->max_size);
		
        return NIGHT_ERROR;
    }
    
    // 检查 bucket_size 是否过大
	if (hinit->bucket_size > night_page_size) 
	{
		dprintf(error_log_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
		dprintf(error_log_fd, "could not build %s, too large %s_bucket_size: %ld\n\n", hinit->name, hinit->name, hinit->bucket_size);
		
        return NIGHT_ERROR;
    }
    
    // 检查每个元素是否能放入单个桶
	for (n = 0; n < nelts; n++) 
	{
        if (names[n].key.data == NULL) 
        {
            continue;
        }
		
		// 计算单个元素所需内存（含对齐）：
		// 桶大小必须至少能容纳一个元素 + 桶末尾的 NULL 终止指针（sizeof(void*)）。
        if (hinit->bucket_size < (NIGHT_HASH_ELT_SIZE(&names[n]) + sizeof(void *)))
        {
        	dprintf(error_log_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        	dprintf(error_log_fd, "could not build %s, you should increase %s_bucket_size: %ld\n\n", hinit->name, hinit->name, hinit->bucket_size);
        	
            return NIGHT_ERROR;
        }
    }
    
	test = malloc(hinit->max_size * sizeof(uint16_t));
    if (test == NULL) 
    {
    	dprintf(error_log_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    	dprintf(error_log_fd, "test = malloc(hinit->max_size * sizeof(uint16_t)) failed\n\n");
    	
        return NIGHT_ERROR;
    }

	// 计算 bucket_size（有效数据空间）
	// 每个桶末尾需要一个 NULL 指针作为链表结束标志，因此有效数据空间要减去 sizeof(void*)
	bucket_size = hinit->bucket_size - sizeof(void *);
	
	// 计算 start（尝试桶数的起始值）
	// 启发式估算：假设每个元素平均占 2 * sizeof(void*) 字节（粗略估计），则每个桶可放 bucket_size / (2*sizeof(void*)) 个元素。
	// 总桶数至少为 nelts / 每桶元素数。
	// 若为 0，则设为 1（至少 1 个桶）。
	start = nelts / (bucket_size / (2 * sizeof(void *)));
    start = start ? start : 1;
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
	dprintf(trace_file_fd, "start=%ld\n\n", start);
	
	// 尝试不同 size（桶数量）直到找到合适的
	for (size = start; size <= hinit->max_size; size++) 
	{
        night_memzero(test, size * sizeof(uint16_t));

        for (n = 0; n < nelts; n++) 
        {
            if (names[n].key.data == NULL) 
            {
                continue;
            }

            key = names[n].key_hash % size;
            // 累加该桶已用空间 test[key] += 元素大小
            len = test[key] + NIGHT_HASH_ELT_SIZE(&names[n]);

            if (len > bucket_size) 
            {
            	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
            	dprintf(trace_file_fd, "if (len > bucket_size)\ngoto next;\n\n");
            	
                goto next;
            }

            test[key] = (uint16_t) len;
        }
		
		// 如果所有元素都能放下，goto found，使用当前 size
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
		dprintf(trace_file_fd, "goto found\n\n");
		
        goto found;

    	next:
			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
			dprintf(trace_file_fd, "next:\n\n");
			
        	continue;
    }
    
    // 若未找到合适 size，使用 max_size 并警告
    // 如果遍历完都没找到合适 size，只能用最大桶数 max_size
    size = hinit->max_size;
    
found:
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
	dprintf(trace_file_fd, "found:\n\n");
	
	// 重置 test：每个桶初始占用 sizeof(void*)（用于末尾的 NULL 终止符）
	for (i = 0; i < size; i++) 
	{
        test[i] = sizeof(void *);
    }
    
    // 重新计算每个桶总大小（含所有元素 + NULL）。
	// 新增检查：单个桶不能超过 night_page_size
	// 如果超限，报错返回。
	for (n = 0; n < nelts; n++) 
	{
        if (names[n].key.data == NULL) {
            continue;
        }

        key = names[n].key_hash % size;
        len = test[key] + NIGHT_HASH_ELT_SIZE(&names[n]);

        if (len > night_page_size) 
        {
        	dprintf(error_log_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        	dprintf(error_log_fd, "could not build %s, you should increase %s_max_size: %ld", hinit->name, hinit->name, hinit->max_size);
        	
            free(test);
            
            return NIGHT_ERROR;
        }

        test[key] = (uint16_t) len;
    }
    
    len = 0;
    
    // 对每个桶大小进行对齐，并计算总内存 len
	for (i = 0; i < size; i++) 
	{
		// 空桶，跳过
        if (test[i] == sizeof(void *)) 
        {
            continue;
        }

        test[i] = (uint16_t) (night_align(test[i], 8));

        len += test[i];
    }
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd, "len=%ld\n\n", len);
    
    // 分配 buckets 数组（桶指针数组）
	if (hinit->hash == NULL) 
	{
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
		dprintf(trace_file_fd, "if (hinit->hash == NULL)\n\n");
		
        hinit->hash = night_pmalloc(hinit->pool, sizeof(night_hash_wildcard_t) + size * sizeof(night_hash_elt_t*));
        
        if (hinit->hash == NULL) 
        {
            free(test);
            return NIGHT_ERROR;
        }

        buckets = (night_hash_elt_t**) ((char*) hinit->hash + sizeof(night_hash_wildcard_t));

    } 
    else 
    {
        buckets = night_pmalloc(hinit->pool, size * sizeof(night_hash_elt_t *));
        if (buckets == NULL) 
        {
            free(test);
            return NIGHT_ERROR;
        }
    }
    
    // 分配所有元素的连续内存
	elts = night_pmalloc(hinit->pool, night_align(len, 8));
    if (elts == NULL) 
    {
        free(test);
        return NIGHT_ERROR;
    }
    
    elts = night_align_ptr(elts, 8);
    
    // 初始化 buckets[i] 指向各桶起始位置
	for (i = 0; i < size; i++) 
	{
        if (test[i] == sizeof(void *)) 
        {
            continue;
        }
		
		// 非空桶的 buckets[i] 指向 elts 当前位置
		// elts 指针前进 test[i] 字节(该桶总大小)
        buckets[i] = (night_hash_elt_t *) elts;
        elts += test[i];
    }
    
	// 重置 test 数组为 0（用于记录各桶当前写入偏移）
	for (i = 0; i < size; i++) 
	{
        test[i] = 0;
    }
    
	// 填充每个桶中的元素
	for (n = 0; n < nelts; n++) 
	{
        if (names[n].key.data == NULL) 
        {
            continue;
        }

        key = names[n].key_hash % size;
        elt = (night_hash_elt_t *) ((char *) buckets[key] + test[key]);

        elt->value = names[n].value;
        elt->len = (uint16_t) names[n].key.len;

        night_strlow(elt->name, names[n].key.data, names[n].key.len);

        test[key] = (uint16_t) (test[key] + NIGHT_HASH_ELT_SIZE(&names[n]));
    }
    
    // 在每个桶末尾添加 NULL 终止符
	for (i = 0; i < size; i++) 
	{
        if (buckets[i] == NULL) 
        {
            continue;
        }

        elt = (night_hash_elt_t *) ((char *) buckets[i] + test[i]);

        elt->value = NULL;
    }
    
    free(test);
    
	hinit->hash->buckets = buckets;
    hinit->hash->size = size;
    
    // 查看 结果
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd, "hash table:\n\n");
    
    for (i = 0; i < size; i++)
    {
		if (buckets[i] == NULL) 
        {
            continue;
        }
        
        elt = buckets[i];
        
        while(elt->value)
        {
			write(trace_file_fd, elt->name, elt->len);
        	dprintf(trace_file_fd, "\n");
        	
        	elt = (night_hash_elt_t*) night_align_ptr(&elt->name[0] + elt->len, sizeof(void *));
        } 
    }
    dprintf(trace_file_fd, "\n");
    
    return NIGHT_OK;
}

uint64_t
night_hash_strlow(char *dst, char *src, size_t n)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
	dprintf(trace_file_fd, "function:\t" "night_hash_strlow\n\n");
	
    uint64_t  key;

    key = 0;

    while (n--) 
    {
        *dst = night_tolower(*src);
        key = night_hash(key, *dst);
        dst++;
        src++;
    }

    return key;
}


