#ifndef _NIGHT_STRING_H_
#define _NIGHT_STRING_H_

#define night_memzero(buf,n) memset(buf,0,n);

#define night_tolower(c) (char) ( ( c >= 'A' && c <= 'Z' ) ? ( c | 0x20 ) : c)

#define night_str_null(str)   (str)->len = 0; (str)->data = NULL

#define night_str_set(str, text)									\
    	(str)->len = sizeof(text) - 1; (str)->data = (char *) text

#define night_cpymem(dst, src, n)   (((char *) memcpy(dst, src, n)) + (n))

char*
night_cpystrn(char *dst, char *src, size_t n);

char* 
night_strlow(char *dst, char *src, size_t n);

int
night_atoi(char *data, size_t len);

char *
night_strcasestrn(char *s1, char *s2, size_t n);

int
night_strncasecmp(char *s1, char *s2, size_t n);

char *
night_strstrn(char *s1, char *s2, size_t n);

off_t
night_atoof(char *line, size_t n);

time_t
night_atotm(char *line, size_t n);

#endif /* _NIGHT_STRING_H_ */
