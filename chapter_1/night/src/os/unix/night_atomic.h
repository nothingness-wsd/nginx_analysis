#ifndef _NIGHT_ATOMIC_H_
#define _NIGHT_ATOMIC_H_

typedef int32_t                     	night_atomic_int_t;
typedef uint32_t                    	night_atomic_uint_t;
typedef volatile night_atomic_uint_t  	night_atomic_t;

#define night_memory_barrier()	__sync_synchronize()

#define night_atomic_cmp_set(lock, old, set)			\
    __sync_bool_compare_and_swap(lock, old, set)
    

#define night_trylock(lock)  	(*(lock) == 0 && night_atomic_cmp_set(lock, 0, 1))
#define night_unlock(lock)    	*(lock) = 0

#endif /* _NIGHT_ATOMIC_H_ */
