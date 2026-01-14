#ifndef _NIGHT_FILE_NAME_H_
#define _NIGHT_FILE_NAME_H_

#define NIGHT_PWD_LEN (1024 * 4)

int
night_get_full_name(night_str_t *name);

int 
night_filename_cmp(char *s1, char *s2, size_t n);

#endif /* _NIGHT_FILE_NAME_H_ */
