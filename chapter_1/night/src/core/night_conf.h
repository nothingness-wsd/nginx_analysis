#ifndef _NIGHT_CONF_H_
#define _NIGHT_CONF_H_

#define NIGHT_MAIN_CONF					0x01000000

#define NIGHT_CONF_OK					NULL
#define NIGHT_CONF_ERROR				(void *) -1

#define NIGHT_CONF_BLOCK_START 			1
#define NIGHT_CONF_BLOCK_DONE  			2
#define NIGHT_CONF_FILE_DONE   			3

#define NIGHT_CONF_NOARGS      0x00000001
#define NIGHT_CONF_TAKE1       0x00000002
#define NIGHT_CONF_TAKE2       0x00000004
#define NIGHT_CONF_TAKE3       0x00000008
#define NIGHT_CONF_TAKE4       0x00000010
#define NIGHT_CONF_TAKE5       0x00000020
#define NIGHT_CONF_TAKE6       0x00000040
#define NIGHT_CONF_TAKE7       0x00000080

#define NIGHT_CONF_BLOCK		0x00000100
#define NIGHT_CONF_FLAG			0x00000200
#define NIGHT_CONF_ANY			0x00000400
#define NIGHT_CONF_1MORE		0x00000800
#define NIGHT_CONF_2MORE		0x00001000

#define NIGHT_CONF_MAX_ARGS		8

#define NIGHT_DIRECT_CONF		0x00010000

#define NIGHT_MAIN_CONF			0x01000000
#define NIGHT_EVENT_CONF		0x02000000

#define NIGHT_HTTP_MAIN_CONF	0x02000000

#define NIGHT_CONF_UNSET       -1
#define NIGHT_CONF_UNSET_UINT	(uint32_t)(-1)
#define NIGHT_CONF_UNSET_PTR   (void *) -1
    
#define night_conf_init_value(conf, default)	\
    if (conf == NIGHT_CONF_UNSET) 				\
    {											\
        conf = default;							\
    }

#define night_conf_init_ptr_value(conf, default)	\
    if (conf == NIGHT_CONF_UNSET_PTR) 				\
    {                                        		\
        conf = default;								\
    }


    
typedef struct night_conf_s				night_conf_t;

struct night_conf_s
{
	char					*name;
	night_array_t			*args;
	
	night_cycle_t			*cycle;
    night_pool_t			*pool;
    night_pool_t			*temp_pool;
    
    void					*ctx;
	uint64_t				module_type;
    uint64_t				cmd_type;
    
    night_conf_file_t		*conf_file;
};

char *
night_conf_parse(night_conf_t *cf, night_str_t *filename);

#endif /* _NIGHT_CONF_H_ */
