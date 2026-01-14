#include "night_core.h"
#include "night_file_name.h"
#include "night_string.h"
#include "night_pool.h"
#include "night_cycle.h"

char night_pwd[NIGHT_PWD_LEN];

night_str_t night_work_directory = 
{
	0,
	night_pwd
};

int
night_get_full_name(night_str_t *name)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_get_full_name\n\n");
    
    char	*path;
    size_t 	prefix_len;
    size_t 	len;
    
    if	(name->data[0] == '/')
    {
    	return NIGHT_OK;
    }
    
    if (night_work_directory.len == 0)
    {
    	night_memzero(night_pwd, NIGHT_PWD_LEN);
    	
    	path = getcwd(night_pwd, NIGHT_PWD_LEN);
    	
    	if (path == NULL)
    	{
    		dprintf(error_log_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    		dprintf(error_log_fd,"getcwd failed while night_get_full_name\n\n");
    	
    		return NIGHT_ERROR;
    	}
    	
    	strcat(night_pwd, "/");
    	len = strlen(night_pwd);
    	
    	night_work_directory.len = len;
    }
    
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"night_work_directory.len=%ld\n", night_work_directory.len);
    dprintf(trace_file_fd,"night_work_directory.data=%s\n\n", night_work_directory.data);
    
    prefix_len = night_work_directory.len;
    len = prefix_len + name->len;
    
    path = night_pmalloc(night_cycle->pool, (len + 1));
    
    strcat(path, night_work_directory.data);
    strcat(path, name->data);
    
    name->data = path;
    name->len = len;
    
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"name.len=%ld\n", name->len);
    dprintf(trace_file_fd,"name=%s\n\n", name->data);
    
    return NIGHT_OK;
}


int 
night_filename_cmp(char *s1, char *s2, size_t n)
{
    uint64_t c1,c2;
    
    while (n)
    {
        c1 = (uint64_t) *s1++;
        c2 = (uint64_t) *s2++;
        
        c1 = night_tolower(c1);
        c2 = night_tolower(c2);
        
        if (c1 == c2) 
        {
            if(c1) 
            {
                n--;
                continue;
            }

            return 0;
        }
        
        if(c1 == 0 || c2 == 0) 
        {
            return c1 - c2;
        }
        
        c1 = (c1 == (uint64_t)'/') ? 0 : c1;
        c2 = (c2 == (uint64_t)'/') ? 0 : c2;
        
        return c1 - c2;
    }
    
    return 0;
}

