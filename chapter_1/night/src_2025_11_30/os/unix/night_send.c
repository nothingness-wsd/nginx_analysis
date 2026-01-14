#include "night_core.h"
#include "night_send.h"

ssize_t
night_unix_send(night_connection_t *c, char *buf, size_t size)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_unix_send\n\n");
    
    return 0;

}
