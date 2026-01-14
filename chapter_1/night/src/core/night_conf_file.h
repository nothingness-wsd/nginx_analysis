#ifndef _NIGHT_CONF_FILE_H_
#define _NIGHT_CONF_FILE_H_


typedef struct night_conf_dump_s night_conf_dump_t;
typedef struct night_conf_file_s night_conf_file_t;

struct night_conf_dump_s
{
    night_str_t				name;
    night_buf_t				*buffer;
};

struct night_conf_file_s
{
    night_file_t			file;
    night_buf_t				*buffer;
    night_buf_t				*dump;
    uint64_t				line;
};

int
night_conf_full_name(night_cycle_t *cycle, night_str_t *name, int conf_prefix);

#endif /* _NIGHT_CONF_FILE_H_ */
