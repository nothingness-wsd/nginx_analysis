#ifndef _NIGHT_WRITEV_CHAIN_H_
#define _NIGHT_WRITEV_CHAIN_H_

#include "night_os.h"

night_chain_t*
night_writev_chain(night_connection_t *c, night_chain_t *in, off_t limit);

night_chain_t *
night_output_chain_to_iovec(night_iovec_t *vec, night_chain_t *in, size_t limit);

ssize_t
night_writev(night_connection_t *c, night_iovec_t *vec);

#endif /* _NIGHT_WRITEV_CHAIN_H_ */
