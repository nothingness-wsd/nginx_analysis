#ifndef _NIGHT_RBTREE_H_
#define _NIGHT_RBTREE_H_

#define night_rbt_is_red(node)            ((node)->color)
#define night_rbt_is_black(node)          (!night_rbt_is_red(node))
#define night_rbt_copy_color(n1, n2)      (n1->color = n2->color)

typedef struct night_rbtree_node_s 	night_rbtree_node_t;
typedef struct night_rbtree_s 		night_rbtree_t;

typedef void (*night_rbtree_insert_pt) 
(night_rbtree_node_t *root, night_rbtree_node_t *node, night_rbtree_node_t *sentinel);

struct night_rbtree_node_s 
{
	int64_t 			key;
	night_rbtree_node_t *left;
    night_rbtree_node_t *right;
    night_rbtree_node_t *parent;
    char 				color;
    char 				data;
};

struct night_rbtree_s 
{
    night_rbtree_node_t     *root;
    night_rbtree_node_t     *sentinel;
    night_rbtree_insert_pt   insert;
};

void
night_rbtree_init(night_rbtree_t *tree, night_rbtree_node_t *s, night_rbtree_insert_pt insert) ;

void      
night_rbtree_sentinel_init(night_rbtree_node_t *node);

void
night_rbt_black(night_rbtree_node_t *node);

void
night_rbtree_insert_timer_value(night_rbtree_node_t *temp, night_rbtree_node_t *node,
    night_rbtree_node_t *sentinel);
    
void
night_rbt_red(night_rbtree_node_t *node);

void
night_rbtree_insert(night_rbtree_t *tree, night_rbtree_node_t *node);

void
night_rbtree_left_rotate(night_rbtree_node_t **root, night_rbtree_node_t *sentinel,
    night_rbtree_node_t *node);
    
void
night_rbtree_right_rotate(night_rbtree_node_t **root, night_rbtree_node_t *sentinel,
    night_rbtree_node_t *node);
    
void
night_rbtree_delete(night_rbtree_t *tree, night_rbtree_node_t *node);  

night_rbtree_node_t*
night_rbtree_min(night_rbtree_node_t *node, night_rbtree_node_t *sentinel);  

#endif /* _NIGHT_RBTREE_H_ */

