#include "night_core.h"
#include "night_linux_init.h"
#include "night_os_io.h"
#include "night_recv.h"
#include "night_send.h"
#include "night_readv_chain.h"
#include "night_linux_sendfile_chain.h"

static night_os_io_t night_linux_io = 
{
    night_unix_recv,
    night_unix_send,
    
    night_readv_chain,
    night_linux_sendfile_chain
};

int
night_os_specific_init()
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "night_os_specific_init\n\n");
	
    night_os_io = night_linux_io;

    return NIGHT_OK;
}
