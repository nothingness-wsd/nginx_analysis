
#ifndef _NIGHT_HASH_H_
#define _NIGHT_HASH_H_

#define night_hash(key, c)   ((uint64_t) key * 31 + c)

// sizeof(void*)：存储 value 指针
// key.len + 2：存储小写 key（+2 是 len 字段）
// 整体对齐到 sizeof(void*)
#define NIGHT_HASH_ELT_SIZE(name)											\
		(sizeof(void *) + night_align((name)->key.len + 2, sizeof(void *)))

typedef struct night_hash_s 			night_hash_t;
typedef struct night_hash_elt_s 		night_hash_elt_t;
typedef struct night_hash_key_s			night_hash_key_t;
typedef struct night_hash_init_s		night_hash_init_t;
typedef struct night_hash_wildcard_s 	night_hash_wildcard_t;

typedef uint64_t (*night_hash_key_pt) (char *data, size_t len);

struct night_hash_init_s
{
	night_hash_t       *hash;
	night_hash_key_pt   key;
	size_t				max_size;
	size_t				bucket_size;
	char				*name;

    night_pool_t       *pool;
    night_pool_t       *temp_pool;
};

struct night_hash_key_s
{
    night_str_t         	key;
    uint64_t        		key_hash;
    void             		*value;
};

// value 指向与该键（name）关联的实际数据
// 作为链表结束的哨兵标志, 哈希桶中的元素在内存中是连续紧凑排列的（无 next 指针）
// 每个桶的最后一个元素之后，会额外放置一个特殊元素：其 value 字段被显式设置为 NULL
// 这个 NULL 不表示有效数据，而是作为遍历终止条件
struct night_hash_elt_s
{
    void				*value;
    uint16_t			len;
    char            	name[1];
};

struct night_hash_s
{
    night_hash_elt_t  	**buckets;
    uint64_t        	size;
};

struct night_hash_wildcard_s
{
    night_hash_t		hash;
    void				*value;
};

struct night_table_elt_s 
{
    uint64_t        	hash;
    night_str_t         key;
    night_str_t         value;
    char           		*lowcase_key;
    night_table_elt_t  	*next;
};

void*
night_hash_find(night_hash_t *hash, uint64_t key, char *name, size_t len);

uint64_t
night_hash_key_lc(char *data, size_t len);

int
night_hash_init(night_hash_init_t *hinit, night_hash_key_t *names, size_t nelts);

uint64_t
night_hash_strlow(char *dst, char *src, size_t n);

#endif /* _NIGHT_HASH_H_ */

