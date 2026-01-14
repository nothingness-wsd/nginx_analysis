#ifndef _NIGHT_CONF_FILE_H_
#define _NIGHT_CONF_FILE_H_

#include "night_file.h"
#include "night_buf.h"

struct night_conf_file_s
{
    night_file_t 	file;
    night_buf_t 	buffer;
    size_t 			line;
};

#endif /* _NIGHT_CONF_FILE_H_ */
