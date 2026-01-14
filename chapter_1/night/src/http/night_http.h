#ifndef _NIGHT_HTTP_H_
#define _NIGHT_HTTP_H_

#include "night_http_conf_ctx.h"
#include "night_http_module_ctx.h"
#include "night_http_core_main_conf.h"
#include "night_http_core_srv_conf.h"
#include "night_http_core_loc_conf.h"
#include "night_http_request.h"
#include "night_http_variables.h"
#include "night_http_variable.h"

#define night_http_conf_get_module_main_conf(cf, module)				\
    ((night_http_conf_ctx_t *) cf->ctx)->main_conf[module.ctx_index]
    
#endif /* _NIGHT_HTTP_H_ */
    
    
