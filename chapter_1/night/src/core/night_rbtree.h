#ifndef _NIGHT_RBTREE_H_
#define _NIGHT_RBTREE_H_

typedef struct night_rbtree_s		night_rbtree_t;
typedef struct night_rbtree_node_s	night_rbtree_node_t;

typedef void (*night_rbtree_insert_pt) (night_rbtree_node_t *root, night_rbtree_node_t *node, night_rbtree_node_t *sentinel);
    
struct night_rbtree_node_s 
{
    uint64_t				key;
    night_rbtree_node_t     *left;
    night_rbtree_node_t     *right;
    night_rbtree_node_t     *parent;
    char					color;
    char					data;
};

struct night_rbtree_s 
{
    night_rbtree_node_t		*root;
    night_rbtree_node_t		*sentinel;
    night_rbtree_insert_pt	insert;
};

#define night_rbt_red(node)				((node)->color = 1)
#define night_rbt_black(node)			((node)->color = 0)

#define night_rbtree_sentinel_init(node)  night_rbt_black(node)

#define night_rbtree_init(tree, s, i)		\
    night_rbtree_sentinel_init(s);			\
    (tree)->root = s;						\
    (tree)->sentinel = s;					\
    (tree)->insert = i
    
#endif /* _NIGHT_RBTREE_H_ */
