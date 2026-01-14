#include "night_core.h"
#include "night_ioctl.h"

int
night_socket_nread(int s, int *n)
{
	int rc;
	
	rc = ioctl(s, FIONREAD, n);
	
	return rc;
}
