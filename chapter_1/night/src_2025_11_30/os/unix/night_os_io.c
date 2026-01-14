#include "night_core.h"
#include "night_os_io.h"
#include "night_recv.h"
#include "night_send.h"
#include "night_readv_chain.h"
#include "night_writev_chain.h"

night_os_io_t night_os_io = 
{
    night_unix_recv,
    night_unix_send,
    
    night_readv_chain,
    night_writev_chain
};

