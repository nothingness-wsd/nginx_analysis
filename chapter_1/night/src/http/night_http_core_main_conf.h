#ifndef _NIGHT_HTTP_CORE_MAIN_CONF_H_
#define _NIGHT_HTTP_CORE_MAIN_CONF_H_

typedef struct night_http_core_main_conf_s night_http_core_main_conf_t;

struct night_http_core_main_conf_s
{
	night_array_t				servers;
	night_array_t				*ports;
	
	night_hash_keys_arrays_t	*variables_keys;
	
	night_array_t				prefix_variables;
};

#endif /* _NIGHT_HTTP_CORE_MAIN_CONF_H_ */
