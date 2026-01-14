#include "night_core.h"
#include "night_string.h"

char*
night_cpystrn(char *dst, char *src, size_t n)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_cpystrn\n\n");
    
    if (n == 0) 
    {
        return dst;
    }

    while(--n) 
    {
        *dst = *src;

        if (*dst == '\0') 
        {
            return dst;
        }

        dst++;
        src++;
    }

    *dst = '\0';

    return dst;
}

char* 
night_strlow(char *dst, char *src, size_t n)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_strlow\n\n");
    
    while(n)
    {
        *dst = night_tolower(*src);
        dst++;
        src++;
        n--;
    }
    
    return dst;
}

int
night_atoi(char *data, size_t len)
{
	int value;
	int cutoff;
	int cutlim;
	
	if (len == 0)
	{
		return NIGHT_ERROR;
	}
	
	cutoff = INT32_MAX / 10;
	cutlim = INT32_MAX % 10;
	
	for (value = 0; len--; data++)
	{
		if((*data) < '0' || (*data) > '9')
		{
			return NIGHT_ERROR;
		}
		
        if (value >= cutoff && (value > cutoff || ((*data) - '0') > cutlim)) 
        {
            return NIGHT_ERROR;
        }
        
        value = value * 10 + ((*data) - '0');
	}
	
	return value;
}

char *
night_strcasestrn(char *s1, char *s2, size_t n)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_strcasestrn\n\n");
    
    uint64_t  c1, c2;

    c2 = (uint64_t) *s2++;
    c2 = (c2 >= 'A' && c2 <= 'Z') ? (c2 | 0x20) : c2;

    do 
    {
        do 
        {
            c1 = (uint64_t) *s1++;

            if (c1 == 0) 
            {
                return NULL;
            }

            c1 = (c1 >= 'A' && c1 <= 'Z') ? (c1 | 0x20) : c1;

        } while (c1 != c2);

    } while (night_strncasecmp(s1, s2, n - 1) != 0);

    return --s1;
}

int
night_strncasecmp(char *s1, char *s2, size_t n)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_strncasecmp\n\n");
    
    uint64_t  c1, c2;

    while (n) 
    {
        c1 = (uint64_t) *s1++;
        c2 = (uint64_t) *s2++;

        c1 = (c1 >= 'A' && c1 <= 'Z') ? (c1 | 0x20) : c1;
        c2 = (c2 >= 'A' && c2 <= 'Z') ? (c2 | 0x20) : c2;

        if (c1 == c2) 
        {
            if (c1) 
            {
                n--;
                continue;
            }

            return 0;
        }

        return c1 - c2;
    }

    return 0;
}

char *
night_strstrn(char *s1, char *s2, size_t n)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_strstrn\n\n");
    
    char  c1, c2;

    c2 = *(char *) s2++;

    do {
        do {
            c1 = *s1++;

            if (c1 == 0) 
            {
                return NULL;
            }

        } while (c1 != c2);

    } while (strncmp(s1, (char *) s2, n - 1) != 0);

    return --s1;
}

off_t
night_atoof(char *line, size_t n)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_atoof\n\n");
    
    off_t  value, cutoff, cutlim;

    if (n == 0) 
    {
        return NIGHT_ERROR;
    }

    cutoff = NIGHT_MAX_OFF_T_VALUE / 10;
    cutlim = NIGHT_MAX_OFF_T_VALUE % 10;

    for (value = 0; n--; line++) 
    {
        if (*line < '0' || *line > '9') 
        {
            return NIGHT_ERROR;
        }

        if (value >= cutoff && (value > cutoff || *line - '0' > cutlim)) 
        {
            return NIGHT_ERROR;
        }

        value = value * 10 + (*line - '0');
    }

    return value;
}

time_t
night_atotm(char *line, size_t n)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "night_atotm\n\n");
    
    time_t  value, cutoff, cutlim;

    if (n == 0) 
    {
        return NIGHT_ERROR;
    }

    cutoff = NIGHT_MAX_TIME_T_VALUE / 10;
    cutlim = NIGHT_MAX_TIME_T_VALUE % 10;

    for (value = 0; n--; line++) 
    {
        if (*line < '0' || *line > '9') 
        {
            return NIGHT_ERROR;
        }

        if (value >= cutoff && (value > cutoff || *line - '0' > cutlim)) 
        {
            return NIGHT_ERROR;
        }

        value = value * 10 + (*line - '0');
    }
    
    return value;
}
