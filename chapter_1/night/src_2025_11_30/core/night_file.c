#include "night_core.h"
#include "night_file.h"

ssize_t 
night_read_file(night_file_t *file, char *buf, size_t size, off_t offset)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_read_file\n\n");
    
    ssize_t n;
    
    n = pread(file->fd, buf, size, offset);
    if(n < 0)
    {
        return n;
    }
    
    file->offset += n;
    
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"file->offset=%ld\n\n",file->offset);
    
    return n;
}
