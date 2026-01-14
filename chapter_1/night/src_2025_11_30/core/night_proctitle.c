#include "night_core.h"
#include "night_proctitle.h"
#include "night_string.h"

#define NIGHT_SETPROCTITLE_PAD (0)

int
night_setproctitle(char *title)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_setproctitle\n\n");
    
	char *p;
    
    night_argv[1] = NULL;
    
    p = night_cpystrn(night_argv[0],"night:", night_argv_last - night_argv[0]);
    
    p = night_cpystrn(p, title, night_argv_last - p);
	
    if(night_argv_last - p) 
    {
        memset(p, NIGHT_SETPROCTITLE_PAD, night_argv_last - p);
    }
    
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"night_argv[0]=%s\n\n", night_argv[0]);
}
