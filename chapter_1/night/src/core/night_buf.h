#ifndef _NIGHT_BUF_H_
#define _NIGHT_BUF_H_

typedef struct night_buf_s				night_buf_t;

struct night_buf_s
{
	char			*pos;
	char			*last;
    off_t			file_pos;
    off_t			file_last;

    char			*start;
    char			*end;  
    
    unsigned		temporary:1;
};

#endif /* _NIGHT_BUF_H_ */
