#include "night_core.h"
#include "night_sock.h"

int 
night_sock_ntop(struct sockaddr *sa, socklen_t socklen, char *text, size_t len, int port)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function:\t" "night_sock_ntop\n\n");
	
	int 					n;
	struct sockaddr_in		*sin;
	char 					*p;
	
	sin = (struct sockaddr_in*) sa;
	p = (char*) &sin->sin_addr;
	
	if(port)
	{
		n = snprintf(text, len, "%u.%u.%u.%u:%d", p[0], p[1], p[2], p[3], ntohs(sin->sin_port));
	} 
	else 
	{
		n = snprintf(text, len, "%u.%u.%u.%u", p[0], p[1], p[2], p[3]);
	}
	
	return n;
}
