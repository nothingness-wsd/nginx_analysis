#ifndef _NIGHT_OPEN_FILE_CACHE_H_
#define _NIGHT_OPEN_FILE_CACHE_H_


// 缓存文件的元信息（metadata）和打开状态
struct night_open_file_info_s
{
	//fd：文件描述符（file descriptor）
	int 							fd;
	
	// 最后一次操作的错误码（如 open、stat 失败时的 errno）
	int                				err;
	
	// 指向一个字符串常量，描述失败的操作名称（如 "open", "stat", "fstat" 等）
	char                    		*failed;
	
	//valid：该缓存条目的有效截止时间（Unix 时间戳）。
	//设置一个 TTL（time-to-live），在此时间之前，缓存信息被认为是有效的，无需重新 stat 或 open。
	//一旦当前时间超过 valid，缓存条目将被重新验证或失效。
	time_t                   		valid;
	
	// 文件的唯一标识符（uniqueidentifier），用于判断文件是否被替换或修改
	// ino_t 是POSIX标准定义的一种无符号整数类型，专门用于存储inode编号
	ino_t              				uniq;
	
	// 文件的最后修改时间
	time_t                   		mtime;
	
	off_t                    		size;
    off_t                    		fs_size;
	
	//min_uses：该缓存条目被使用的最小次数阈值。
	//只有被访问次数 ≥ min_uses 的文件才会被缓存。
	//此字段用于跟踪或判断是否满足缓存条件。
	uint8_t               			min_uses;
	
	// 表示仅测试文件是否存在/可访问，而不实际打开文件
	unsigned                 		test_only:1;
	
	unsigned                 		is_dir:1;
    unsigned                 		is_file:1;
    unsigned                 		is_link:1;
    unsigned                 		is_exec:1;
};

//用于实现 nginx 的“打开文件缓存”（open file cache）功能
//该功能主要用于缓存已打开文件的元信息（如 stat 信息、文件描述符等），以避免频繁地执行系统调用（如 open、stat 等），从而提升性能，尤其在处理大量静态文件请求时非常关键
struct night_open_file_cache_s
{

};

int
night_open_cached_file(night_open_file_cache_t *cache, night_str_t *name, night_open_file_info_t *of, night_pool_t *pool);

int
night_file_info_wrapper(night_str_t *name, night_open_file_info_t *of, struct stat *fi);

int
night_open_and_stat_file(night_str_t *name, night_open_file_info_t *of);

int
night_open_file_wrapper(night_str_t *name, night_open_file_info_t *of, int mode, int create, int access);
	
#endif /* _NIGHT_OPEN_FILE_CACHE_H_ */
