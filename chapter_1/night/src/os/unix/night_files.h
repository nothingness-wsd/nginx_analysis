#ifndef _NIGHT_FILES_H_
#define _NIGHT_FILES_H_


#define NIGHT_INVALID_FILE	-1
#define NIGHT_FILE_ERROR	-1

ssize_t
night_read_file(night_file_t *file, char *buf, size_t size, off_t offset);

#endif /* _NIGHT_FILES_H_ */
