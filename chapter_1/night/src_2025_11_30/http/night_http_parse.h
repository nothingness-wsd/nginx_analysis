#ifndef _NIGHT_HTTP_PARSE_H_
#define _NIGHT_HTTP_PARSE_H_

int
night_http_parse_request_line(night_http_request_t *r, night_buf_t *b);

int
night_http_parse_header_line(night_http_request_t *r, night_buf_t *b);

#endif /* _NIGHT_HTTP_PARSE_H_ */
