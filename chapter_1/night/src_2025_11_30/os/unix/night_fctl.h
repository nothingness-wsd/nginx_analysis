#ifndef _NIGHT_FCTL_H_
#define _NIGHT_FCTL_H_

int
night_nonblocking(int fd);

int
night_async(int fd);

int
night_fdCloexec(int fd);

#endif /* _NIGHT_FCTL_H_ */
