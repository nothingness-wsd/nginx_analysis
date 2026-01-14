#ifndef _NIGHT_H_
#define _NIGHT_H_

int
night_get_options(int argc, char *const *argv);

static int
night_save_argv(night_cycle_t *cycle, int argc, char *const *argv);

static int
night_process_options(night_cycle_t *cycle);

#endif /* _NIGHT_H_ */
