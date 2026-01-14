#ifndef _NIGHT_FILE_H_
#define _NIGHT_FILE_H_

typedef struct night_path_s		night_path_t;
typedef struct night_file_s		night_file_t;

struct night_path_s
{

};

struct night_file_s 
{
    int							fd;
	night_str_t					name;
    struct stat					info;
    
    off_t						offset;
};

int
night_get_full_name(night_pool_t *pool, night_str_t *prefix, night_str_t *name);

#endif /* _NIGHT_FILE_H_ */
