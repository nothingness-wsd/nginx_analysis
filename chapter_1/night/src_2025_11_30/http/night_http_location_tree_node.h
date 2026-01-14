#ifndef _NIGHT_HTTP_LOCATION_TREE_NODE_H_
#define _NIGHT_HTTP_LOCATION_TREE_NODE_H_

struct night_http_location_tree_node_s
{
    night_http_location_tree_node_t 	*left;
    night_http_location_tree_node_t 	*right;
    night_http_location_tree_node_t 	*tree;

    night_http_core_loc_conf_t 			*exact;
    night_http_core_loc_conf_t 			*inclusive;

    size_t 								len;
    char 								name[1];
};

#endif /* _NIGHT_HTTP_LOCATION_TREE_NODE_H_ */
