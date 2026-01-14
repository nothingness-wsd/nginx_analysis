#ifndef _NIGHT_HTTP_VARIABLE_H_
#define _NIGHT_HTTP_VARIABLE_H_

#define night_http_null_variable  { night_null_string, NULL, NULL, 0, 0, 0 }

typedef night_variable_value_t  night_http_variable_value_t;

typedef struct night_http_variable_s night_http_variable_t;

typedef void (*night_http_set_variable_pt) (night_http_request_t *r,
    night_http_variable_value_t *v, uintptr_t data);
    
typedef int (*night_http_get_variable_pt) (night_http_request_t *r,
    night_http_variable_value_t *v, uintptr_t data);
    
struct night_http_variable_s
{
    night_str_t						name;
    night_http_set_variable_pt      set_handler;
    night_http_get_variable_pt      get_handler;
    uintptr_t						data;
    int                    			flags;
    int                    			index;
};



#endif /* _NIGHT_HTTP_VARIABLE_H_ */
