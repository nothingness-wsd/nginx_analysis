#ifndef _NIGHT_HTTP_MODULE_H_
#define _NIGHT_HTTP_MODULE_H_

#include "night_http_location_tree_node.h"
#include "night_http_request.h"

#define NIGHT_HTTP_SRV_CONF_OFFSET offsetof(night_http_conf_t,srv_conf)
#define NIGHT_HTTP_LOC_CONF_OFFSET offsetof(night_http_conf_t,loc_conf)

#define NIGHT_INET_ADDRSTRLEN (sizeof("255.255.255.255") - 1)
#define NIGHT_SOCKADDR_STRLEN (NIGHT_INET_ADDRSTRLEN + sizeof(":65535") - 1)

typedef int (*night_http_output_header_filter_pt)(night_http_request_t *r);
typedef int (*night_http_output_body_filter_pt)(night_http_request_t *r, night_chain_t *chain);
typedef int (*night_http_request_body_filter_pt)(night_http_request_t *r, night_chain_t *chain);

struct night_http_conf_s
{
    void** main_conf;
    void** srv_conf;
    void** loc_conf;
};

int
night_http_block(night_conf_t *cf, night_command_t *cmd, void *conf);

int 
night_http_init_location_trees(night_conf_t *cf, night_http_conf_t *hc);

int
night_http_init_locations(night_conf_t *cf, night_http_core_loc_conf_t *plc);

int
night_http_init_static_location_trees(night_conf_t *cf, night_http_core_loc_conf_t *plc);

int
night_http_cmp_locations(const night_queue_t *one, const night_queue_t *two);

int
night_http_join_exact_locations(night_conf_t *cf, night_queue_t *locations);

void
night_http_create_locations_list(night_queue_t *locations, night_queue_t *q);

night_http_location_tree_node_t*
night_http_create_locations_tree(night_conf_t *cf, night_queue_t *locations, size_t prefix);

int 
night_http_optimize_servers(night_conf_t *cf, night_http_conf_t *hc);

int
night_http_init_listening(night_conf_t *cf, night_http_port_t *port);

night_listening_t*
night_http_add_listening(night_conf_t *cf, night_http_port_t *port);

night_listening_t*
night_create_listening(night_conf_t *cf, struct sockaddr *sockaddr, socklen_t socklen);

int 
night_sock_ntop(struct sockaddr *sa, socklen_t socklen, char *text, size_t len, int port);

int
night_http_init_headers_in_hash(night_conf_t *cf, night_http_core_main_conf_t *cmcf);

int
night_http_init_phases(night_conf_t *cf, night_http_core_main_conf_t *cmcf);

int
night_http_init_phase_handlers(night_conf_t *cf, night_http_core_main_conf_t *cmcf);

extern night_http_output_header_filter_pt  night_http_top_header_filter;
extern night_http_output_body_filter_pt    night_http_top_body_filter;

#endif /* _NIGHT_HTTP_MODULE_H_ */
