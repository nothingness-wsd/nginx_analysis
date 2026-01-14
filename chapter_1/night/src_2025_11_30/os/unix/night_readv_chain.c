#include "night_core.h"
#include "night_readv_chain.h"

ssize_t
night_readv_chain(night_connection_t *c, night_chain_t *chain, off_t limit)
{
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd, "function:\t" "night_readv_chain\n\n" );
    
    return 0;
}
