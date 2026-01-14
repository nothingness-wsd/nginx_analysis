#include "night_core.h"

char *
night_cpystrn(char *dst, char *src, size_t n)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function:\t" "night_cpystrn\n\n");
	
    if (n == 0) 
    {
        return dst;
    }

    while (--n) 
    {
        *dst = *src;

        if (*dst == '\0') 
        {
            return dst;
        }

        dst++;
        src++;
    }

    *dst = '\0';

	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function night_cpystrn:\t" "return\n\n");
	
    return dst;
}

char *
night_pstrdup(night_pool_t *pool, night_str_t *src)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function:\t" "%s\n\n", __func__);
	
    char  *dst;

    dst = night_pnalloc(pool, src->len);
    if (dst == NULL) 
    {
        return NULL;
    }

    memcpy(dst, src->data, src->len);

	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	write(trace_file_fd, dst, src->len);
	dprintf(trace_file_fd,"\nfunction %s:\t" "return\n\n", __func__);
	
    return dst;
}

void
night_str_rbtree_insert_value(night_rbtree_node_t *temp,
    night_rbtree_node_t *node, night_rbtree_node_t *sentinel)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "%s\n\n", __func__);
	
    night_str_node_t		*n, *t;
    night_rbtree_node_t		**p;

    for ( ;; ) 
    {
        n = (night_str_node_t *) node;
        t = (night_str_node_t *) temp;

        if (node->key != temp->key) 
        {
            p = (node->key < temp->key) ? &temp->left : &temp->right;

        } else if (n->str.len != t->str.len) 
        {
            p = (n->str.len < t->str.len) ? &temp->left : &temp->right;

        } else 
        {
            p = (memcmp(n->str.data, t->str.data, n->str.len) < 0)
                 ? &temp->left : &temp->right;
        }

        if (*p == sentinel) 
        {
            break;
        }

        temp = *p;
    }

    *p = node;
    node->parent = temp;
    node->left = sentinel;
    node->right = sentinel;
    night_rbt_red(node);
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function %s:\t" "return\n\n", __func__);
}

void
night_strlow(char *dst, char *src, size_t n)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "%s\n\n", __func__);
	
    while (n) 
    {
        *dst = night_tolower(*src);
        
        dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(trace_file_fd, "%c\n\n", *dst);
        
        dst++;
        src++;
        n--;
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function %s:\t" "return\n\n", __func__);
}

int
night_atoi(char *line, size_t n)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "%s\n\n", __func__);
	
	uint64_t  value, cutoff, cutlim;
	
	if (n == 0) 
	{
        return NIGHT_ERROR;
    }
	
    cutoff = NIGHT_MAX_INT_T_VALUE / 10;
    cutlim = NIGHT_MAX_INT_T_VALUE % 10;
    
	for (value = 0; n--; line++) 
	{
        if (*line < '0' || *line > '9') 
        {
            return NIGHT_ERROR;
        }

        if (value >= cutoff && (value > cutoff || *line - '0' > cutlim)) 
        {
            return NIGHT_ERROR;
        }

        value = value * 10 + (*line - '0');
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function %s:\t" "return value(%ld)\n\n", __func__, value);
	
    return value;
}

