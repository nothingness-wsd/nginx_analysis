#ifndef _NIGHT_OPEN_FILE_H_
#define _NIGHT_OPEN_FILE_H_

typedef struct night_open_file_s night_open_file_t;

struct night_open_file_s 
{
    int						fd;
    night_str_t				name;
};

#endif /* _NIGHT_OPEN_FILE_H_ */
