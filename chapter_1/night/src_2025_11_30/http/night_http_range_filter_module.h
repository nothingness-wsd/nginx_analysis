#ifndef _NIGHT_HTTP_RANGE_FILTER_MODULE_H_
#define _NIGHT_HTTP_RANGE_FILTER_MODULE_H_

typedef struct night_http_range_filter_ctx_s night_http_range_filter_ctx_t;

int
night_http_range_header_filter_init(night_conf_t *cf);

int
night_http_range_body_filter_init(night_conf_t *cf);

static int
night_http_range_header_filter(night_http_request_t *r);

static int
night_http_range_body_filter(night_http_request_t *r, night_chain_t *in);

struct night_http_range_filter_ctx_s
{

};

#endif /* _NIGHT_HTTP_RANGE_FILTER_MODULE_H_ */

