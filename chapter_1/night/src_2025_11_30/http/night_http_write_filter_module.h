#ifndef _NIGHT_HTTP_WRITE_FILTER_MODULE_H_
#define _NIGHT_HTTP_WRITE_FILTER_MODULE_H_

int night_http_write_filter_init(night_conf_t *cf);

int
night_http_write_filter(night_http_request_t *r, night_chain_t *in);

#endif /* _NIGHT_HTTP_WRITE_FILTER_MODULE_H_ */
