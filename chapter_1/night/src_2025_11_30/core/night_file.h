#ifndef _NIGHT_FILE_H_
#define _NIGHT_FILE_H_

#define night_file_info_n			"stat()"
#define night_open_file_n			"open()"

#define NIGHT_FILE_ERROR			-1
#define NIGHT_INVALID_FILE         	-1

#define night_file_uniq(sb)			(sb)->st_ino
#define night_file_mtime(sb)       	(sb)->st_mtime
#define night_file_size(sb)        	(sb)->st_size

#define night_is_dir(sb)           	(S_ISDIR((sb)->st_mode))
#define night_is_file(sb)          	(S_ISREG((sb)->st_mode))
#define night_is_link(sb)          	(S_ISLNK((sb)->st_mode))

//S_IXUSR：标准 POSIX 名称，表示 User eXecute permission（文件所有者的执行权限位）
#define night_is_exec(sb)          	(((sb)->st_mode & S_IXUSR) == S_IXUSR)

//估算文件在文件系统中实际占用的磁盘空间大小
// (sb)->st_blocks：这是 struct stat 中的一个字段，表示文件占用的 512 字节块数（注意：这是 POSIX 标准定义的单位，不是文件系统块大小）
//(sb)->st_blocks * 512：将块数转换为字节数，得到文件在磁盘上实际占用的 物理空间（以字节计）。
//(sb)->st_size：这是文件的 逻辑大小（logical size），即文件内容的实际字节数
//什么磁盘占用会大于逻辑大小？
//因为文件系统以“块”为单位分配空间。即使文件只有 1 字节，也可能占用一个完整的块
// (sb)->st_blksize：这是文件系统推荐的 I/O 块大小
//8 * (sb)->st_blksize：这里取了一个“容差范围”（tolerance），即允许磁盘占用比逻辑大小多出最多 8 个文件系统块
//因为某些文件系统或实现中，st_blocks 可能包含间接块、元数据、日志开销等，导致 st_blocks * 512 明显大于 st_size。
//但 Nginx 认为，如果超出太多（比如超过 8 个块），那可能不是“正常”的文件空间占用，可能是异常或特殊文件（如设备文件、管道等），此时不应信任 st_blocks * 512。 
#define night_file_fs_size(sb)												\
    (((sb)->st_blocks * 512 > (sb)->st_size									\
     && (sb)->st_blocks * 512 < (sb)->st_size + 8 * (sb)->st_blksize)		\
     ? (sb)->st_blocks * 512 : (sb)->st_size)
    
#define night_open_file(name, mode, create, access)							\
    open((const char *) name, mode|create, access)

struct night_file_s
{
    night_str_t filename;
    int 		fd;
    struct stat info;
    off_t 		offset;
};

ssize_t 
night_read_file(night_file_t *file, char *buf, size_t size, off_t offset);

#endif /* _NIGHT_FILE_H_ */
