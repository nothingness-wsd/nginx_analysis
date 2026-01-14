#ifndef _NIGHT_STRING_H_
#define _NIGHT_STRING_H_

#define night_memzero(buf, n)       (void) memset(buf, 0, n)

#define night_tolower(c)	(char) ((c >= 'A' && c <= 'Z') ? (c | 0x20) : c)
#define night_toupper(c)	(char) ((c >= 'a' && c <= 'z') ? (c & ~0x20) : c)

#define night_cpymem(dst, src, n)   (((char *) memcpy(dst, src, n)) + (n))

#define night_string(str)	{ sizeof(str) - 1, (char *) str }

#define night_str_set(str, text)                                               \
		(str)->len = sizeof(text) - 1; (str)->data = (char *) text

#define night_null_string     { 0, NULL }

typedef struct night_str_s 			night_str_t;

typedef struct night_str_node_s 	night_str_node_t;

struct night_str_s
{
    size_t      len;
    char     	*data;
};

struct night_str_node_s
{
    night_rbtree_node_t			node;
    night_str_t					str;
};

char *
night_cpystrn(char *dst, char *src, size_t n);

char *
night_pstrdup(night_pool_t *pool, night_str_t *src);

void
night_str_rbtree_insert_value(night_rbtree_node_t *temp,
	night_rbtree_node_t *node, night_rbtree_node_t *sentinel);
	
void
night_strlow(char *dst, char *src, size_t n);

int
night_atoi(char *line, size_t n);

#endif /* _NIGHT_STRING_H_ */
