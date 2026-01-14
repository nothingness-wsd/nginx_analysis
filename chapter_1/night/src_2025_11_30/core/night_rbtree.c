#include "night_core.h"
#include "night_rbtree.h"

void
night_rbtree_init(night_rbtree_t *tree, night_rbtree_node_t *s, night_rbtree_insert_pt insert)    
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"function:\t" "night_rbtree_init\n\n");
    
    night_rbtree_sentinel_init(s); 
    
	(tree)->root = s;                                                         
    (tree)->sentinel = s;                                                     
    (tree)->insert = insert; 
}                                       
      
void      
night_rbtree_sentinel_init(night_rbtree_node_t *node)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"function:\t" "night_rbtree_sentinel_init\n\n");
    
    night_rbt_black(node);
    
    node->key = 0;
	node->left = NULL;
	node->right = NULL;
	node->parent = NULL;
    node->data = 0;
}                                                

void
night_rbt_black(night_rbtree_node_t *node)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"function:\t" "night_rbt_black\n\n");
    
    node->color = 0;
}

void
night_rbtree_insert_timer_value(night_rbtree_node_t *temp, night_rbtree_node_t *node,
    night_rbtree_node_t *sentinel)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"function:\t" "night_rbtree_insert_timer_value\n\n");
    
    night_rbtree_node_t  **p;
    
    for ( ;; ) 
    {
		p = (node->key - temp->key) < 0 ? &temp->left : &temp->right;

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
}    

void
night_rbt_red(night_rbtree_node_t *node)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"function:\t" "night_rbt_red\n\n");
    
    node->color = 1;
}

void
night_rbtree_insert(night_rbtree_t *tree, night_rbtree_node_t *node)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"function:\t" "night_rbtree_insert\n\n");
    
    night_rbtree_node_t **root, *temp, *sentinel;
    
    // a binary tree insert 
    root = &tree->root;
    sentinel = tree->sentinel;
    
    if (*root == sentinel) 
    {
    	node->parent = NULL;
		node->left = sentinel;
        node->right = sentinel;
        
        night_rbt_black(node);
        *root = node;
        
        return;
    }
    
    tree->insert(*root, node, sentinel);
    
   	// re-balance tree 
    while (node != *root && night_rbt_is_red(node->parent)) 
    {
    	if (node->parent == node->parent->parent->left) 
    	{
    		temp = node->parent->parent->right;
    		
    		if (night_rbt_is_red(temp)) 
    		{
                night_rbt_black(node->parent);
                night_rbt_black(temp);
                night_rbt_red(node->parent->parent);
                node = node->parent->parent;
            }
            else 
            {
                if (node == node->parent->right) 
                {
                    node = node->parent;
                    night_rbtree_left_rotate(root, sentinel, node);
                }

                night_rbt_black(node->parent);
                night_rbt_red(node->parent->parent);
                night_rbtree_right_rotate(root, sentinel, node->parent->parent);
            }    
    	}
    	else 
    	{
            temp = node->parent->parent->left;

            if (night_rbt_is_red(temp)) 
            {
                night_rbt_black(node->parent);
                night_rbt_black(temp);
                night_rbt_red(node->parent->parent);
                node = node->parent->parent;

            } else 
            {
                if (node == node->parent->left) 
                {
                    node = node->parent;
                    night_rbtree_right_rotate(root, sentinel, node);
                }

                night_rbt_black(node->parent);
                night_rbt_red(node->parent->parent);
                night_rbtree_left_rotate(root, sentinel, node->parent->parent);
            }
        }
    }
    
    night_rbt_black(*root);
}

void
night_rbtree_left_rotate(night_rbtree_node_t **root, night_rbtree_node_t *sentinel,
    night_rbtree_node_t *node)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"function:\t" "night_rbtree_left_rotate\n\n");
    
    night_rbtree_node_t *temp;

    temp = node->right;
    node->right = temp->left;

    if (temp->left != sentinel) 
    {
        temp->left->parent = node;
    }

    temp->parent = node->parent;

    if (node == *root) 
    {
        *root = temp;

    } else if (node == node->parent->left) 
    {
        node->parent->left = temp;

    } else {
        node->parent->right = temp;
    }

    temp->left = node;
    node->parent = temp;
}

void
night_rbtree_right_rotate(night_rbtree_node_t **root, night_rbtree_node_t *sentinel,
    night_rbtree_node_t *node)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"function:\t" "night_rbtree_right_rotate\n\n");
    
    night_rbtree_node_t *temp;

    temp = node->left;
    node->left = temp->right;

    if (temp->right != sentinel) 
    {
        temp->right->parent = node;
    }

    temp->parent = node->parent;

    if (node == *root) 
    {
        *root = temp;

    } else if (node == node->parent->right) 
    {
        node->parent->right = temp;

    } else 
    {
        node->parent->left = temp;
    }

    temp->right = node;
    node->parent = temp;
}
    
void
night_rbtree_delete(night_rbtree_t *tree, night_rbtree_node_t *node)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"function:\t" "night_rbtree_delete\n\n");
    
    int red;
    night_rbtree_node_t **root, *sentinel, *subst, *temp, *w;

    // a binary tree delete 
    root = &tree->root;
    sentinel = tree->sentinel;

    if (node->left == sentinel) 
    {
        temp = node->right;
        subst = node;

    } else if (node->right == sentinel) 
    {
        temp = node->left;
        subst = node;

    } else 
    {
        subst = night_rbtree_min(node->right, sentinel);
        temp = subst->right;
    }

    if (subst == *root) 
    {
        *root = temp;
        night_rbt_black(temp);

        // DEBUG stuff
        node->left = NULL;
        node->right = NULL;
        node->parent = NULL;
        node->key = 0;

        return;
    }

    red = night_rbt_is_red(subst);

    if (subst == subst->parent->left) 
    {
        subst->parent->left = temp;

    } else 
    {
        subst->parent->right = temp;
    }

    if (subst == node) 
    {
        temp->parent = subst->parent;

    } else 
    {
        if (subst->parent == node) 
        {
            temp->parent = subst;

        } else 
        {
            temp->parent = subst->parent;
        }

        subst->left = node->left;
        subst->right = node->right;
        subst->parent = node->parent;
        night_rbt_copy_color(subst, node);

        if (node == *root) 
        {
            *root = subst;

        } else 
        {
            if (node == node->parent->left) 
            {
                node->parent->left = subst;
            } else 
            {
                node->parent->right = subst;
            }
        }

        if (subst->left != sentinel) 
        {
            subst->left->parent = subst;
        }

        if (subst->right != sentinel) {
            subst->right->parent = subst;
        }
    }

    // DEBUG stuff 
    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;
    node->key = 0;

    if (red) 
    {
        return;
    }

    // a delete fixup 

    while (temp != *root && night_rbt_is_black(temp)) 
    {
        if (temp == temp->parent->left) 
        {
            w = temp->parent->right;

            if (night_rbt_is_red(w)) 
            {
                night_rbt_black(w);
                night_rbt_red(temp->parent);
                night_rbtree_left_rotate(root, sentinel, temp->parent);
                w = temp->parent->right;
            }

            if (night_rbt_is_black(w->left) && night_rbt_is_black(w->right)) 
            {
                night_rbt_red(w);
                temp = temp->parent;

            } else {
                if (night_rbt_is_black(w->right)) 
                {
                    night_rbt_black(w->left);
                    night_rbt_red(w);
                    night_rbtree_right_rotate(root, sentinel, w);
                    w = temp->parent->right;
                }

                night_rbt_copy_color(w, temp->parent);
                night_rbt_black(temp->parent);
                night_rbt_black(w->right);
                night_rbtree_left_rotate(root, sentinel, temp->parent);
                temp = *root;
            }

        } else {
            w = temp->parent->left;

            if (night_rbt_is_red(w)) {
                night_rbt_black(w);
                night_rbt_red(temp->parent);
                night_rbtree_right_rotate(root, sentinel, temp->parent);
                w = temp->parent->left;
            }

            if (night_rbt_is_black(w->left) && night_rbt_is_black(w->right)) {
                night_rbt_red(w);
                temp = temp->parent;

            } else {
                if (night_rbt_is_black(w->left)) {
                    night_rbt_black(w->right);
                    night_rbt_red(w);
                    night_rbtree_left_rotate(root, sentinel, w);
                    w = temp->parent->left;
                }

                night_rbt_copy_color(w, temp->parent);
                night_rbt_black(temp->parent);
                night_rbt_black(w->left);
                night_rbtree_right_rotate(root, sentinel, temp->parent);
                temp = *root;
            }
        }
    }

    night_rbt_black(temp);
}

night_rbtree_node_t*
night_rbtree_min(night_rbtree_node_t *node, night_rbtree_node_t *sentinel)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"function:\t" "night_rbtree_min\n\n");
    
    while (node->left != sentinel) 
    {
        node = node->left;
    }

    return node;
}

