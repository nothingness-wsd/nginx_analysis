#ifndef _NIGHT_CHANNEL_H_
#define _NIGHT_CHANNEL_H_ 

#define NIGHT_CMD_OPEN_CHANNEL   1
#define NIGHT_CMD_CLOSE_CHANNEL  2
#define NIGHT_CMD_QUIT           3
#define NIGHT_CMD_TERMINATE      4
#define NIGHT_CMD_REOPEN         5

struct night_channel_s 
{
    uint32_t 	command;
    pid_t 		pid;
    int 		slot;
    int 		fd;
};

int
night_write_channel(int s, night_channel_t *ch, size_t size);

int
night_read_channel(int s, night_channel_t *ch, size_t size);

void
night_close_channel(int *channel);

int
night_add_channel_event(int fd, int event, night_event_handler_pt handler);

void
night_channel_handler(night_event_t *e);

#endif /* _NIGHT_CHANNEL_H_ */
