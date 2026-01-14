#include "night_core.h"
#include "night_listening.h"
#include "night_core_module.h"
#include "night_module.h"
#include "night_cycle.h"
#include "night_fctl.h"
#include "night_connection.h"
#include "night_event.h"

int
night_clone_listening(night_listening_t *ls)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_clone_listening\n\n");
    
    night_listening_t 	old_ls;
    night_core_conf_t 	*ccf;
    int 				n;
    
    if(ls->worker != 0)
    {
    	return NIGHT_OK;
    }
    
    old_ls = *ls;
    
    ccf = (night_core_conf_t*) night_get_conf(night_cycle->conf_ctx, night_core_module);
    
    for (n = 1; n < ccf->worker_processes; n++)
    {
    	/* create a socket for each worker process */
    	ls = night_array_push(&night_cycle->listening);
    	if (ls == NULL)
    	{
    		return NIGHT_ERROR;
    	}
    	
		*ls = old_ls;
        ls->worker = n;
    }
    
    return NIGHT_OK;
}

int
night_open_listening_sockets()
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_open_listening_sockets\n\n");
    
    int 				tries;
    int 				failed;
    night_listening_t 	*ls;
    int 				i;
    int 				n;
    int 				s;
    int 				rc;
    int 				resueaddr;
    int 				reuseport;
    
    resueaddr = 1;
    reuseport = 1;
    
	ls = night_cycle->listening.elts;
	n= night_cycle->listening.nelts;
    	
    for (tries = 0; tries < 5; tries++)
    {
    	failed = 0;
    	
		for (i = 0; i < n; i++)
		{
			if (ls[i].fd != -1) 
            {
                continue;
            }
            
            s = socket(ls[i].sockaddr->sa_family, ls[i].type, 0);
            if (s == -1)
            {
            	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            	dprintf(error_log_fd,"socket(%s) failed\n\n", ls[i].addr_text.data);
            	
            	failed = 1;
            	continue;
            }
            
            // SO_REUSEPORT
            rc = setsockopt(s, SOL_SOCKET, SO_REUSEPORT, &reuseport, sizeof(int));
			if (rc == -1)
            {
            	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            	dprintf(error_log_fd,"%s setsockopt(SO_REUSEPORT) failed\n\n", ls[i].addr_text.data);
            	
            	close(s);
            	failed = 1;
            	continue;
			}
            
            // SO_REUSEADDR
            if (ls[i].type != SOCK_DGRAM)
            {
            	rc = setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&resueaddr,sizeof(int));
				if ( rc == -1 )
                {
                	dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
                    dprintf(error_log_fd,"%s" 
                    	"setsockopt(%d, SOL_SOCKET, SO_REUSEADDR, %d, sizeof(int)) failed\n\n",
                     	ls[i].addr_text.data, s, resueaddr);
                    
                    close(s);
                    failed = 1;
                    continue;
                }
            } 
               
			rc = night_nonblocking(s);
			if (rc == -1) 
			{
				dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
				dprintf(error_log_fd, "%s night_nonblocking failed\n\n", ls[i].addr_text.data);
                	
				close(s);
				failed = 1;
				continue;
			}
            
            rc = bind(s, ls[i].sockaddr, ls[i].socklen);    
			if (rc == -1)
            {
                dprintf(error_log_fd, "%s bind failed\n", ls[i].addr_text.data);
                dprintf(error_log_fd, "errno=%d:\n",errno);
                dprintf(error_log_fd, "%s\n\n",strerror(errno));

                close(s);
                failed = 1;
                continue;
            }
            dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            dprintf(trace_file_fd,"%s bind completed\n\n", ls[i].addr_text.data);
            
            rc = listen(s, ls[i].backlog);
			if (rc == -1)
            {
                dprintf(error_log_fd, "%s listen failed\n", ls[i].addr_text.data);
				dprintf(error_log_fd, "errno=%d:\n",errno);
                dprintf(error_log_fd, "%s\n\n",strerror(errno));
                
                close(s);
                failed = 1;
                continue;
            }
            dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
            dprintf(trace_file_fd,"%s listen() completed\n\n", ls[i].addr_text.data);
            
            ls[i].fd = s;
		}
    }
    
	if(failed)
    {
        return NIGHT_ERROR;
    }
    
    return NIGHT_OK;
}

int
night_configure_listening_sockets()
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"function:\t" "night_configure_listening_sockets\n\n");

	night_listening_t 	*ls;
	int 				i;
	int 				value;
	int 				rc;
	
	ls = night_cycle->listening.elts;
	for (i = 0; i < night_cycle->listening.nelts; i++)
	{
		value = 1;
		if (setsockopt(ls[i].fd, SOL_SOCKET, SO_KEEPALIVE, &value, sizeof(int)) == -1)
		{
			dprintf(error_log_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
			dprintf(error_log_fd, "%s setsockopt(SO_KEEPALIVE, %d) failed, ignored\n\n", 
									ls[i].addr_text.data, value);
			
		}
		
		value = 5;
		if (setsockopt(ls[i].fd, IPPROTO_TCP, TCP_DEFER_ACCEPT, &value, sizeof(int))== -1)
		{
			dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
			dprintf(error_log_fd,"%s setsockopt(TCP_DEFER_ACCEPT, %d) failed,ignored\n\n", 		
									ls[i].addr_text.data, value);

			continue;
		}                
        
		if (listen(ls[i].fd, ls[i].backlog) == -1) 
		{
			dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
			dprintf(error_log_fd,"listen() to %s ,backlog %d failed\n\n", 
							ls[i].addr_text.data, ls[i].backlog);
			
			return NIGHT_ERROR;
		}
	}

	return NIGHT_OK;
}

int
night_close_listening_sockets()
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_close_listening_sockets\n\n");
    
    int 				i;
    night_listening_t 	*ls;
    night_connection_t  *c;
    int 				rc = 0;

    ls = night_cycle->listening.elts;
    for (i = 0; i < night_cycle->listening.nelts; i++) 
    {
        c = ls[i].connection;
        if (c) 
        {
            if (c->read->active) 
            {
				night_del_event(c->read, NIGHT_READ_EVENT, 0);   
            }

            night_free_connection(c);

            c->fd =  -1;
        }
		
		if (ls[i].fd != -1)
		{
			rc = close(ls[i].fd);
			if (rc == -1)
			{
				dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
				dprintf(error_log_fd,"close %s failed \n\n", ls[i].addr_text.data);
			}
			
			ls[i].fd = -1;
		}
    }

    night_cycle->listening.nelts = 0;
    
    return NIGHT_OK;
}
