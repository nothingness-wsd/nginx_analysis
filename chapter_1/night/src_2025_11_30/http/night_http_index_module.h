#ifndef _NIGHT_HTTP_INDEX_MODULE_H_
#define _NIGHT_HTTP_INDEX_MODULE_H_

#include "night_array.h"

typedef struct night_http_index_loc_conf_s 	night_http_index_loc_conf_t;
typedef struct night_http_index_s 			night_http_index_t;

struct night_http_index_s
{
	night_str_t 				name;
	night_array_t             	*lengths;
	night_array_t             	*values;
};


struct night_http_index_loc_conf_s
{
    night_array_t 	indices;
    size_t 			max_index_len;
};

void*
night_http_index_create_loc_conf(night_conf_t *cf);

int 
night_http_index(night_conf_t *cf, night_command_t *cmd, void *conf);

int
night_http_index_init(night_conf_t *cf);

int
night_http_index_handler(night_http_request_t *r);

#endif /* _NIGHT_HTTP_INDEX_MODULE_H_ */
