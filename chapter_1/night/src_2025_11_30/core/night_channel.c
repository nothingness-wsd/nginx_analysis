#include "night_core.h"
#include "night_channel.h"
#include "night_string.h"
#include "night_connection.h"
#include "night_cycle.h"
#include "night_event.h"
#include "night_process.h"

int
night_write_channel(int s, night_channel_t *ch, size_t size)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_write_channel\n\n");
    
    struct iovec        iov[1];
    struct msghdr       msg;
    ssize_t             n;
    
	union 
	{
        struct cmsghdr cm;
        char space[CMSG_SPACE(sizeof(int))];
    } cmsg;
    
	if (ch->fd == -1) 
	{
		dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd,"if(ch->fd == -1) \n\n");
		
        msg.msg_control = NULL;
        msg.msg_controllen = 0;
	}
	else
	{
		dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        dprintf(trace_file_fd,"msg.msg_control\n\n" );
        
        msg.msg_control = (caddr_t) &cmsg;
        msg.msg_controllen = sizeof(cmsg);

        night_memzero(&cmsg, sizeof(cmsg));

        cmsg.cm.cmsg_len = CMSG_LEN(sizeof(int));
        cmsg.cm.cmsg_level = SOL_SOCKET;
        cmsg.cm.cmsg_type = SCM_RIGHTS;

        memcpy(CMSG_DATA(&cmsg.cm), &ch->fd, sizeof(int));
	}
	
	msg.msg_flags = 0;
	
	iov[0].iov_base = (char *) ch;
    iov[0].iov_len = size;

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;

    n = sendmsg(s, &msg, 0);

    if (n == -1) 
    {
        if (errno == EAGAIN) 
        {
            return NIGHT_AGAIN;
        }

		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(error_log_fd, "sendmsg() failed \n\n");
		
        return NIGHT_ERROR;
    }
    
    return NIGHT_OK;
}

int
night_read_channel(int s, night_channel_t *ch, size_t size)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_read_channel\n\n");
    
    struct 	iovec iov[1];
    int 	fd;
    struct 	msghdr msg;
    ssize_t n;
    
	union 
	{
        struct cmsghdr cm;
        char space[CMSG_SPACE(sizeof(int))];
    } cmsg;
    
	iov[0].iov_base = (char*)ch;
    iov[0].iov_len = size;
    
	msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    
	msg.msg_control = (caddr_t) &cmsg;
    msg.msg_controllen = sizeof(cmsg);
    
    n = recvmsg(s, &msg, 0);
	if (n == -1) 
	{
        if (errno == EAGAIN) 
        {
            return NIGHT_AGAIN;
        }
		
		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(error_log_fd, "recvmsg() failed\n\n");
        
        return NIGHT_ERROR;
    }

    if (n == 0) 
    {
    	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(error_log_fd,"recvmsg() returned zero\n\n");
        
        return NIGHT_ERROR;
    }

    if ((size_t) n < sizeof(night_channel_t)) 
    {
    	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(error_log_fd,"recvmsg() returned not enough data: %ld\n\n", n);
        
        return NIGHT_ERROR;
    }
    
	if (ch->command == NIGHT_CMD_OPEN_CHANNEL) 
	{
		if (cmsg.cm.cmsg_len < (socklen_t)CMSG_LEN(sizeof(int))) 
		{
			dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            dprintf(error_log_fd,"recvmsg() returned too small ancillary data\n\n");
            
            return NIGHT_ERROR;
        }

        if (cmsg.cm.cmsg_level != SOL_SOCKET || cmsg.cm.cmsg_type != SCM_RIGHTS)
        {
        	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(error_log_fd,"recvmsg() returned invalid ancillary data "
					"level %d or type %d",
					cmsg.cm.cmsg_level, cmsg.cm.cmsg_type);
            
            return NIGHT_ERROR;
        }

        memcpy(&ch->fd, CMSG_DATA(&cmsg.cm), sizeof(int));
    }

    if (msg.msg_flags & (MSG_TRUNC|MSG_CTRUNC)) 
    {
    	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(error_log_fd,"recvmsg() truncated data\n\n");
    }
    
    return n;
}

void
night_close_channel(int *channel)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_close_channel\n\n");
    
    if (close(channel[0]) == -1) 
    {
		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
		dprintf(error_log_fd,"close() channel failed\n\n");
    }

    if (close(channel[1]) == -1) 
    {
		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
		dprintf(error_log_fd,"close() channel failed\n\n");
    }
}

int
night_add_channel_event(int fd, int event, night_event_handler_pt handler)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_add_channel_event\n\n");
    
    night_connection_t 	*c;
	night_event_t 		*ev;
	night_event_t 		*rev;
	night_event_t 		*wev;
	int rc;
    
    c = night_get_connection(fd);
    if (c == NULL)
    {
		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        dprintf(error_log_fd,"night_get_connection failed\n\n" );
        
        return NIGHT_ERROR;
    }
    
    c->pool = night_cycle->pool;
    
	rev = c->read;
    wev = c->write;
    
	rev->channel = 1;
    wev->channel = 1;
    
	ev = (event == NIGHT_READ_EVENT) ? rev : wev;
    
    ev->handler = handler;
    
    rc = night_add_event(ev, event, EPOLLET);
	if (rc != NIGHT_OK) 
	{
		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        dprintf(error_log_fd,"night_add_event failed\n\n" );
        
		night_free_connection(c);
		return NIGHT_ERROR;
	}
    
    return NIGHT_OK;
}

void
night_channel_handler(night_event_t *e)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_channel_handler\n\n");
    
    night_connection_t 		*c;
    int 					n;
    night_channel_t 		ch;
    
    c = e->data;
    
    for( ; ; )
    {
    	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd,"night_read_channel\n\n");
    	
    	n = night_read_channel(c->fd, &ch, sizeof(night_channel_t));
    	
    	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd,"%d = night_read_channel\n\n", n);
    	
		if (n == NIGHT_ERROR) 
		{
			night_del_conn(c, 0);

            night_close_connection(c);
            
            return;
        }
        
		if (n == NIGHT_AGAIN) 
		{
            return;
        }
        
        dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(trace_file_fd,"ch.command=%d\n\n", ch.command);
        
		switch(ch.command) 
		{
        	case NIGHT_CMD_QUIT:
            	night_quit = 1;
            	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            	dprintf(trace_file_fd,"night_quit = 1;\n\n");
            	break;

        	case NIGHT_CMD_TERMINATE:
            	night_terminate = 1;
            	break;

        	case NIGHT_CMD_REOPEN:
        	    night_reopen = 1;
        	    break;

       		case NIGHT_CMD_OPEN_CHANNEL:

            	night_processes[ch.slot].pid = ch.pid;
            	night_processes[ch.slot].channel[0] = ch.fd;
            
            	break;

        	case NIGHT_CMD_CLOSE_CHANNEL:

            	if(close(night_processes[ch.slot].channel[0]) == -1) 
            	{
            		dprintf(error_log_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            		dprintf(error_log_fd, "close() channel failed\n\n");
            	}

            	night_processes[ch.slot].channel[0] = -1;
            	break;
        }
    } 
}
