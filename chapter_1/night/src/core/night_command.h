#ifndef _NIGHT_COMMAND_H_
#define _NIGHT_COMMAND_H_


#define night_null_command  { night_null_string, 0, NULL, 0, 0, NULL }

typedef struct night_command_s night_command_t;

struct night_command_s 
{
    night_str_t             name;
    uint64_t            	type;
    char               		*(*set)(night_conf_t *cf, night_command_t *cmd, void *conf);
    // 配置指针数组的偏移
    off_t            		conf;
    // 配置结构体内字段偏移
    off_t            		offset;
    // 附加处理数据
    void					*post;
};

#endif /* _NIGHT_COMMAND_H_ */
