#ifndef _NIGHT_HASH_H_
#define _NIGHT_HASH_H_

#define NIGHT_HASH_SMALL			1
#define NIGHT_HASH_LARGE			2

#define NIGHT_HASH_LARGE_ASIZE		16384
#define NIGHT_HASH_LARGE_HSIZE		10007

int
night_hash_keys_array_init(night_hash_keys_arrays_t *ha, int type);

#endif /* _NIGHT_HASH_H_ */
