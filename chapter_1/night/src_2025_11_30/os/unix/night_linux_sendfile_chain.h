#ifndef _NIGHT_LINUX_SENDFILE_CHAIN_H_
#define _NIGHT_LINUX_SENDFILE_CHAIN_H_

#define NIGHT_SENDFILE_MAXSIZE  2147483647L

night_chain_t *
night_linux_sendfile_chain(night_connection_t *c, night_chain_t *in, off_t limit);

#endif /* _NIGHT_LINUX_SENDFILE_CHAIN_H_ */
