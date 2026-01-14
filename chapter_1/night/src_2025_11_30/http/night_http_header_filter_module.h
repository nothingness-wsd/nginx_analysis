#ifndef _NIGHT_HTTP_HEADER_FILTER_MODULE_
#define _NIGHT_HTTP_HEADER_FILTER_MODULE_

int
night_http_header_filter_init(night_conf_t *cf);

int
night_http_header_filter(night_http_request_t *r);

#endif /* _NIGHT_HTTP_HEADER_FILTER_MODULE_ */
