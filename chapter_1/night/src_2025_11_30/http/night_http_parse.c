#include "night_core.h"
#include "night_http_parse.h"
#include "night_http_request.h"
#include "night_buf.h"
#include "night_hash.h"

uint32_t  http_uri_usual[] = 
{
    0x00000000, /* 0000 0000 0000 0000  0000 0000 0000 0000 */

                /* ?>=< ;:98 7654 3210  /.-, +*)( '&%$ #"!  */
    0x7fff37d6, /* 0111 1111 1111 1111  0011 0111 1101 0110 */

                /* _^]\ [ZYX WVUT SRQP  ONML KJIH GFED CBA@ */
    0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */


                /*  ~}| {zyx wvut srqp  onml kjih gfed cba` */
    0x7fffffff, /* 0111 1111 1111 1111  1111 1111 1111 1111 */

    0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
    0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
    0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
    0xffffffff  /* 1111 1111 1111 1111  1111 1111 1111 1111 */
};

char lowcase[] =
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0-\0\0" "0123456789\0\0\0\0\0\0"
        "\0abcdefghijklmnopqrstuvwxyz\0\0\0\0\0"
        "\0abcdefghijklmnopqrstuvwxyz\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";

int
night_http_parse_request_line(night_http_request_t *r, night_buf_t *b)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"function:\t" "night_http_parse_request_line\n\n");
    
    // 枚举，代表解析时解析到某一种情况
    enum
    {
		//代表 开始解析
    	sw_start = 0,
		//代表 目前在解析请求方法（GET/POST等）
    	sw_method,
    	// 代表 请求方法 解析完成 处于请求方法后，URI之前的空格
    	sw_spaces_before_uri,
    	// 代表 在解析URI 时，遇到 /，当前正处于 / 后第一个字符的位置
    	sw_after_slash_in_uri,
    	// 检查 / 后的内容 
    	sw_check_uri,
    	// 代表 目前在解析 http 协议
    	sw_http_09,
    	// 代表  HTTP 协议部分的第一个字符是H，要检验第二个字符
    	sw_http_H,
    	// 代表  HTTP 协议部分的第二个字符是T，要检验第三个字符
    	sw_http_HT,
		// 代表  HTTP 协议部分的第三个字符是T，要检验第四个字符
    	sw_http_HTT,
		// 代表  HTTP 协议部分的第四个字符是P，要检验之后的内容
    	sw_http_HTTP,
    	// 代表 当前处于HTTP 协议主版本号的位置，解析主版本号
		sw_first_major_digit,
		// 代表 当前处于主版本号后的位置
		sw_major_digit,
		// 代表 当前处于次版本号的位置
		sw_first_minor_digit,
		// 代表 当前处于次版本号后的位置
		sw_minor_digit,
    	//代表  当前处于请求行末尾的换行符位置，这个换行符是请求行结束的标识
    	sw_almost_done
    } state;
    
    char 					*p;
    char					ch;
    char					*m;
    char					c;
    
    state = r->state;
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd, "state=%d\n\n", state);
    
    // 逐个字符解析
    for (p = b->pos; p < b->last; p++) 
    {
    	ch = *p;
    	
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    	dprintf(trace_file_fd, "ch=%c\n\n", ch);
    	
		switch (state) 
    	{
			// start to parse request line
			case sw_start:
				dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
				dprintf(trace_file_fd, "case sw_start:\n\n");
			
				// record the starting position of the request line
				r->request_start = p;
    	       		
				// 验证第一个字符是否合法：必须是大写字母（A-Z）
				// HTTP 方法名应全为大写（如 GET、POST）
				if (ch < 'A' || ch > 'Z') 
				{
					return NIGHT_HTTP_PARSE_INVALID_METHOD;
				}
				
				// start to parse request method
				state = sw_method;
				break;
				
			case sw_method:
				dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
				dprintf(trace_file_fd, "case sw_method:\n\n");
				
				// 遇到空格，说明方法名结束	
            	if (ch == ' ') 
            	{
					// Record the position where the request method end
					r->method_end = p;
                	m = r->request_start;
                	
					// 根据方法长度   判断具体方法
        			switch(p - m)
        			{
        				case 3:
        					dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        					dprintf(trace_file_fd, "case 3:\n\n");
        					
							if (strncmp(m, "GET ", 4) == 0) 
							{
                       			r->method = NIGHT_HTTP_GET;
                       			
                       			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
                       			dprintf(trace_file_fd, "r->method = NIGHT_HTTP_GET;\n\n" );
                       			
                        		break;
                    		}
                    		
							if (strncmp(m, "PUT ", 4) == 0) 
							{
                        		r->method = NIGHT_HTTP_PUT;
                        		
								dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
                       			dprintf(trace_file_fd, "r->method = NIGHT_HTTP_PUT;\n\n" );
                       			
                        		break;
                    		}
                    		
                    		break;
                    		
						case 4:	
							dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        					dprintf(trace_file_fd, "case 4:\n\n");
        					
							if (m[1] == 'O') 
        					{
        						if(strncmp(m, "POST ", 5) == 0)
        						{
        							r->method = NIGHT_HTTP_POST;
        							
									dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
                       				dprintf(trace_file_fd, "r->method = NIGHT_HTTP_POST;\n\n" );
                       			
        							break;
        						}
                    		}
                    			
        					break;
        					
						default:
        					dprintf(error_log_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        					dprintf(error_log_fd, "request method is unknown\n\n" );
        					
							dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        					dprintf(trace_file_fd, "return NIGHT_HTTP_PARSE_INVALID_METHOD;\n\n");
        					return NIGHT_HTTP_PARSE_INVALID_METHOD;
        			}
        			
        			// method parse completed, next to parse uri
					state = sw_spaces_before_uri;
					
					dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
					dprintf(trace_file_fd, "state = sw_spaces_before_uri;\n\n" );
					
        			break;
            	}
            	
				if (ch < 'A' || ch > 'Z') 
				{
					dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
					dprintf(trace_file_fd, "return NIGHT_HTTP_PARSE_INVALID_METHOD;\n\n" );
					
                	return NIGHT_HTTP_PARSE_INVALID_METHOD;
            	}
        		
        		break;
        		
			// space before URI
        	case sw_spaces_before_uri:	
        		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        		dprintf(trace_file_fd, "case sw_spaces_before_uri:\n\n");
        		
				// 若遇到 /，说明是相对 URI（如 /index.html），记录 URI 起始位置并进入路径检查状态
        		if (ch == '/') 
        		{
					r->uri_start = p;
                	state = sw_after_slash_in_uri;
                	
                	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
                	dprintf(trace_file_fd, "state = sw_after_slash_in_uri;\n\n");
                	
                	break;
        		}
        		
        		break;
        		
			// 状态机进入“斜杠后的 URI 初始检查阶段”。
			// 表示刚刚遇到一个 /，现在要检查其后跟的字符
        	case sw_after_slash_in_uri:	
        		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        		dprintf(trace_file_fd, "case sw_after_slash_in_uri:\n\n");
        		
        		// 这是一个位图查表法，用于快速判断字符是否为“常规字符”
        		// usual 是一个全局数组，
        		// 每个元素是 32 位整数，覆盖 ASCII 0~127。
				// 每一位代表一个字符是否“常见”（可安全跳过的字符，如字母、数字、-_.~ 等）。
				// ch >> 5：取高 3 位作为索引
				// ch & 0x1f：取低 5 位
				// (1U << (ch & 0x1f))：生成掩码
				// & 操作：测试该位是否为 1
				if (http_uri_usual[ch >> 5] & (1U << (ch & 0x1f))) 
				{
					// 如果是常规字符  to check uri
                	state = sw_check_uri;
                	
                	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
                	dprintf(trace_file_fd, "state = sw_check_uri;\n\n");
                	
                	break;
            	}
            	
				// 当前字符不是“常规字符”，需要特殊处理。
				// 使用 switch(ch) 分支 判断具体类型。
            	switch (ch) 
            	{
					// 遇到空格，表示 URI 结束，即将进入协议版本部分。
            		case ' ':
            			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
            			dprintf(trace_file_fd, "case ' ':\n\n" );
            			
            			// 记录 URI 结束位置
						r->uri_end = p;
						// 准备解析  http Protocol(HTTP/1.1)
                		state = sw_http_09;
                		
						dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
                		dprintf(trace_file_fd, "state = sw_http_09;\n\n");
                	
                		break;
                		
					default:
            			// ch < 0x20：ASCII 控制字符（如 \t, \n, \b）
						// ch == 0x7f：DEL 字符
						if (ch < 0x20 || ch == 0x7f) 
						{
							dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
							dprintf(trace_file_fd, "return NIGHT_HTTP_PARSE_INVALID_REQUEST;\n\n" );
							
                    		return NIGHT_HTTP_PARSE_INVALID_REQUEST;
                		}
                		state = sw_check_uri;
                		
                		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
                		dprintf(trace_file_fd, "state = sw_check_uri;\n\n");
                		
            			break;	
            		
            	}
            	
            	break;
            	
			case sw_http_09:
				dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
				dprintf(trace_file_fd, "case sw_http_09:\n\n");
				
				switch (ch) 
        		{
        			// 允许 URI 和协议之间有多个空格
        			case ' ':
        				dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        				dprintf(trace_file_fd, "case ' ':\n\n");
        				
                		break;
                	
                	// 缺少协议版本，默认为 9	
					case CR:
                		r->http_minor = 9;
                		state = sw_almost_done;
                		break;
                		
            		case LF:
                		r->http_minor = 9;
                		
                		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
                		dprintf(trace_file_fd, "goto done;\n\n");
                		
                		goto done;	
					
					// 当前字符是 'H'，可能是 HTTP/1.1 的开头
					case 'H':
						dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
						dprintf(trace_file_fd, "case 'H':\n\n");
						
						// 记录 HTTP/1.1 字符串的起始位置
                		r->http_protocol.data = p;
                		// 转移到下一个状态 sw_http_H,开始逐字符验证 HTTP/ 是否完整匹配
                		state = sw_http_H;
                		
                		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
                		dprintf(trace_file_fd, "state = sw_http_H;\n\n");
                		
                		break;	
                		
                	default:
                		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
                		dprintf(trace_file_fd, "return NIGHT_HTTP_PARSE_INVALID_REQUEST;\n\n");
                		
                		return NIGHT_HTTP_PARSE_INVALID_REQUEST;	
				}
				
        		break;
        		
        	// 协议部分的第一个字符是H，检查协议的第二个字符	
			case sw_http_H:	
				dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
				dprintf(trace_file_fd, "case sw_http_H:\n\n" );
					
        		switch (ch) 
        		{
        			// 如果当前字符是 'T'，说明前两个字符是 HT
        			case 'T':
        				state = sw_http_HT;
        				
        				dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        				dprintf(trace_file_fd, "state = sw_http_HT;\n\n");
                		
                		break;
                		
                	default:
                	
                		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
                		dprintf(trace_file_fd, "return NIGHT_HTTP_PARSE_INVALID_REQUEST;\n\n" );
                		
                		return NIGHT_HTTP_PARSE_INVALID_REQUEST;
        		}
        		
        		break;
        	
        	//	协议部分的第2个字符是T，检查协议的第3个字符	
			case sw_http_HT:
				dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
				dprintf(trace_file_fd, "case sw_http_HT:\n\n" );
					
				switch (ch) 
        		{
        			// 如果当前字符是 'T'，说明前三个字符是 HTT
        			case 'T':
                		state = sw_http_HTT;
                		
                		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
                		dprintf(trace_file_fd, "state = sw_http_HTT;\n\n");
                		
                		break;
                		
                	default:
                		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
                		dprintf(trace_file_fd, "NIGHT_HTTP_PARSE_INVALID_REQUEST;\n\n");
                		
                		return NIGHT_HTTP_PARSE_INVALID_REQUEST;
        		}
        		break;
        	
        	// 协议部分的第3个字符是T，检查协议的第4个字符	
        	case sw_http_HTT:
        		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        		dprintf(trace_file_fd, "case sw_http_HTT:\n\n");
        		
        		switch (ch) 
        		{
        			// 如果当前字符是 'P'，说明前四个字符是 P
        			case 'P':
						state = sw_http_HTTP;
						
						dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
						dprintf(trace_file_fd, "case 'P':\n\n" );
						
                		break;
                		
        			default:
        				dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        				dprintf(trace_file_fd, "return NIGHT_HTTP_PARSE_INVALID_REQUEST;\n\n");
        				
        				return NIGHT_HTTP_PARSE_INVALID_REQUEST;
        		}
        		break;
        	
        	case sw_http_HTTP:
        		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        		dprintf(trace_file_fd, "case sw_http_HTTP:\n\n" );
        		
        		switch (ch) 
        		{
        			// 如果当前字符是 '/'，说明完整匹配了 HTTP/，符合 HTTP 协议标准
        			case '/':
        				dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        				dprintf(trace_file_fd, "case '/':\n\n");
        				
        				// 将状态切换为 sw_first_major_digit
						// 表示：“现在开始解析 HTTP 版本的主版本号”
						// 例如：HTTP/1.1 中的 1（主版本）
						state = sw_first_major_digit;
						
						dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
						dprintf(trace_file_fd, "state = sw_first_major_digit;\n\n");
						
                		break;
                		
        			default:
        				dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        				dprintf(trace_file_fd, "return NIGHT_HTTP_PARSE_INVALID_REQUEST;\n\n");
        				
        				return NIGHT_HTTP_PARSE_INVALID_REQUEST;
        		}
        		
        		break;
        	
        	// first digit of major HTTP version 
			// 表示：已经成功匹配了 HTTP/
			// 现在要读取主版本号的第一位数字
        	case sw_first_major_digit:
        		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        		dprintf(trace_file_fd, "case sw_first_major_digit:\n\n");
        		
        		// 非法字符
				if (ch < '1' || ch > '9') 
				{
					dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
					dprintf(trace_file_fd, "if (ch < '1' || ch > '9') \n\n");
					
                	return NIGHT_HTTP_PARSE_INVALID_REQUEST;
            	}
            	
            	// 获取主版本号
            	r->http_major = ch - '0';
            	
            	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
            	dprintf(trace_file_fd, "r->http_major=%d\n\n", r->http_major);
            	
            	// 检查主版本号是否大于 1
				// 如果是（如 2, 3, 9），则返回 NIGHT_HTTP_PARSE_INVALID_VERSION
				// 只支持 HTTP/1.x 和 HTTP/0.9
				if (r->http_major > 1) 
				{
					dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
					dprintf(trace_file_fd, "if (r->http_major > 1)\n\n");
					
                	return NIGHT_HTTP_PARSE_INVALID_VERSION;
            	}
            	// 设置下一个状态为 sw_major_digit
				// 表示：“现在开始处理主版本号的后续数字（如果有）”
				state = sw_major_digit;
            	
            	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
            	dprintf(trace_file_fd, "state = sw_major_digit;\n\n");
            	
        		break;
			
			// major HTTP version or dot 
        	case sw_major_digit:
        		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        		dprintf(trace_file_fd, "case sw_major_digit:\n\n");
        		
        		// .（结束主版本，进入次版本）
        		if (ch == '.') 
        		{
        			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        			dprintf(trace_file_fd, "if (ch == '.') \n\n");
        			
					state = sw_first_minor_digit;
					
					dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
					dprintf(trace_file_fd, "state = sw_first_minor_digit;\n\n");
					
                	break;
        		}
        		
        		// 非法
				if (ch < '0' || ch > '9') 
				{
					dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
					dprintf(trace_file_fd, "if (ch < '0' || ch > '9') \n\n");
					
                	return NIGHT_HTTP_PARSE_INVALID_REQUEST;
            	}
            	
            	r->http_major = r->http_major * 10 + (ch - '0');
            	// 只支持 HTTP/1.x 和 HTTP/0.9
				// 不支持 HTTP/2, HTTP/10, HTTP/11 等
				if (r->http_major > 1) 
				{
					dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
					dprintf(trace_file_fd, "if (r->http_major > 1) \n\n");
				
                	return NIGHT_HTTP_PARSE_INVALID_VERSION;
            	}
        		
        		break;
        		
        	// first digit of minor HTTP version
        	case sw_first_minor_digit:
        		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        		dprintf(trace_file_fd, "case sw_first_minor_digit:\n\n");
        		
				if (ch < '0' || ch > '9') 
				{
					dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
					dprintf(trace_file_fd, "if (ch < '0' || ch > '9') \n\n" );
					
                	return NIGHT_HTTP_PARSE_INVALID_REQUEST;
            	}
            	
				r->http_minor = ch - '0';
				
				dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
				dprintf(trace_file_fd, "r->http_minor=%d\n\n", r->http_minor);
				
            	state = sw_minor_digit;
            	
            	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
            	dprintf(trace_file_fd, "state = sw_minor_digit;\n\n");
            
        		break;
        		
			// minor HTTP version or end of request line
        	case sw_minor_digit:
        		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        		dprintf(trace_file_fd, "case sw_minor_digit:\n\n");
        		
        		if (ch == CR) 
        		{
        			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        			dprintf(trace_file_fd, "if (ch == CR)\n\n");
        			
					state = sw_almost_done;
					
					dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
					dprintf(trace_file_fd, "state = sw_almost_done;\n\n");
					
                	break;
        		}
        		
        		if (ch == LF) 
        		{
        			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        			dprintf(trace_file_fd, "if (ch == LF) \n\n");
        			
        			goto done;
        			
        			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        			dprintf(trace_file_fd, "goto done;\n\n");
        		}
        		
				if (ch < '0' || ch > '9') 
				{
					dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
					dprintf(trace_file_fd, "if (ch < '0' || ch > '9') \n\n");
					
                	return NIGHT_HTTP_PARSE_INVALID_REQUEST;
            	}

            	if (r->http_minor > 99) 
            	{
            		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
            		dprintf(trace_file_fd, "if (r->http_minor > 99) \n\n" );
            		
                	return NIGHT_HTTP_PARSE_INVALID_REQUEST;
            	}
            	r->http_minor = r->http_minor * 10 + (ch - '0');
            	
            	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
            	dprintf(trace_file_fd, "r->http_minor=%d\n\n", r->http_minor);
            	
        		break;
        		
			// end of request line
        	case sw_almost_done:
        		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        		dprintf(trace_file_fd, "case sw_almost_done:\n\n");
        		
        		//request_end 指向的是 CR 的位置，这样 request_end - request_start 就是请求行内容的长度
            	r->request_end = p - 1;
            	
            	switch (ch) 
            	{
            		case LF:
            			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
            			dprintf(trace_file_fd, "case LF:\n\n");
            			
                		goto done;
                		
            		default:
            			
                		return NIGHT_HTTP_PARSE_INVALID_REQUEST;
            	}
            			
			default:
				return NIGHT_ERROR; 		
    	}
    }
    
    // 请求行未解析完成，记录当前位置和状态
    // 返回 NIGHT_AGAIN，需要再次读取数据
	b->pos = p;
    r->state = state;
    
    return NIGHT_AGAIN;
    
done:
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
	dprintf(trace_file_fd, "done:\n\n");
	
	// 记录当前位置
    b->pos = p + 1;
	
	// request_end 暂时为请求行结束位置
    if (r->request_end == NULL) 
    {
        r->request_end = p;
    }
	
	// 获得版本号
    r->http_version = r->http_major * 1000 + r->http_minor;
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd, "r->http_version=%d\n\n", r->http_version);
    
    // 请求行解析完成，state 重置为 sw_start
    r->state = sw_start;

	// 0.9版本只支持 GET 请求方法
    if (r->http_version == 9 && r->method != NIGHT_HTTP_GET) 
    {
    	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    	dprintf(trace_file_fd, " if (r->http_version == 9 && r->method != NIGHT_HTTP_GET)\n\n");
    	
        return NIGHT_HTTP_PARSE_INVALID_09_METHOD;
    }

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
	dprintf(trace_file_fd, "return NIGHT_OK\n\n");
	
    return NIGHT_OK;

}

int
night_http_parse_header_line(night_http_request_t *r, night_buf_t *b)
{
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd,"function:\t" "night_http_parse_header_line\n\n");
    
    char					*p;
    char					ch;
    char					c;
    uint64_t				hash;
    int						i;
    
    // 枚举 代表各个解析阶段
	enum 
	{
		// 解析开始
        sw_start = 0,
        // 解析 header_name
        sw_name,
        // header_name 结束，位于 冒号后， value 前 的空格位置
        sw_space_before_value,
        // 开始解析值
        sw_value,
        // 解析值时遇到 空格
        sw_space_after_value,
        //sw_ignore_line,
        // 一个 header 行结束
        sw_almost_done,
        // header 部分结束
        sw_header_almost_done
    } state;
    
    state = r->state;
    
    dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
    dprintf(trace_file_fd, "state=%d\n\n", state);
    
	// 主循环：逐字节处理
	for (p = b->pos; p < b->last; p++)
	{
		ch = *p;
		
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
		dprintf(trace_file_fd, "ch=%c\n\n", ch);
		
		switch (state) 
		{
			// first char
			case sw_start:
				dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
				dprintf(trace_file_fd, "case sw_start:\n\n");
				
				// 记录 header 名的起始位置。假设 header 有效（除非遇到非法字符）。
            	r->header_name_start = p;
            	r->invalid_header = 0;
            	
            	// 处理第一个字符 ch：
            	switch (ch) 
            	{
					//如果第一个字符是 \r，说明可能是空行（header 结束），进入 sw_header_almost_done，等待 \n。
					case CR:
						dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
						dprintf(trace_file_fd, "case CR:\n\n");
						
						// 记录结束位置
                		r->header_end = p;
                		state = sw_header_almost_done;
                		
                		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
                		dprintf(trace_file_fd, "state = sw_header_almost_done\n\n");
                		
                		break;
                	
                	// 如果是 \n，直接认为 header 结束（空行），跳转到 header_done。	
            		case LF:
            			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
            			dprintf(trace_file_fd, "case LF:\n\n");
            			
                		r->header_end = p;
                		
                		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
                		dprintf(trace_file_fd, "goto header_done;\n\n");
                		
                		goto header_done;
                	
                	// 否则进入 sw_name 状态，开始解析 header 名。	
            		default:
            			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
            			dprintf(trace_file_fd, "default:\nstate = sw_name;\n\n");
            			
                		state = sw_name;
                		
                		// 尝试转小写：
                		c = lowcase[ch];
                		
                		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
                		dprintf(trace_file_fd, "c = lowcase[ch];\nc=%c\n\n", c);
                		
                		// 如果 lowcase[ch] != 0，说明是合法字符（字母、数字、-）。
						if (c) 
						{
							dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
							dprintf(trace_file_fd, "if (c)\n\n");
							
							// 初始化哈希
                    		hash = night_hash(0, c);
                    		
                    		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
                    		dprintf(trace_file_fd, "hash=%ld\n\n", hash);
                    		
                    		// 存入 lowcase_header[0]，索引 i=1
                    		// i 是一个局部变量，代表下一个要写入 lowcase_header 的索引位置。
                    		r->lowcase_header[0] = c;
                    		i = 1;
                    		
                    		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
                    		dprintf(trace_file_fd, "r->lowcase_header=%s\n\n", r->lowcase_header);
                    		
                    		break;
                		}
                		
                		//处理下划线 _：	
						if (ch == '_') 
						{
							dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
							dprintf(trace_file_fd, "if (ch == '_')\n\n");
							
                        	hash = night_hash(0, ch);
                        	r->lowcase_header[0] = ch;
                        	i = 1;
                        	
                        	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
                        	dprintf(trace_file_fd, "r->lowcase_header=%s\n\n", r->lowcase_header);
                        	
                    		break;
                		}
                		
						// 非法起始字符：
						// 控制字符（<= 0x20）、DEL（0x7f）、冒号 : 不能作为 header 名的首字符。立即返回错误。
						if (ch <= 0x20 || ch == 0x7f || ch == ':') 
						{
							dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
							dprintf(trace_file_fd, "if (ch <= 0x20 || ch == 0x7f || ch == ':')\n\n");
							
                    		r->header_end = p;
                    		return NIGHT_HTTP_PARSE_INVALID_HEADER;
                		}
                		
                		//其他非法字符：
						hash = 0;
                		i = 0;
                		
						// 标记为无效，但继续解析（为了兼容性，不立即中断）
                		r->invalid_header = 1;
                		
                		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
                		dprintf(trace_file_fd, "r->invalid_header = 1;\n\n");
                		
                		break;
            	}
            	break;
            
            //  解析 header 名
            case sw_name:
            	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
            	dprintf(trace_file_fd, "case sw_name:\n\n");
            	
            	c = lowcase[ch];
            	
				if (c) 
				{
                	hash = night_hash(hash, c);
					
					dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
					dprintf(trace_file_fd, "hash=%ld\n\n", hash);
                    			
                	r->lowcase_header[i++] = c;
                	
                	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
                	dprintf(trace_file_fd, "r->lowcase_header=");
                	write(trace_file_fd, r->lowcase_header, i);
                	dprintf(trace_file_fd, "\n\n");
                	
                	// i 循环递增
                	i &= (NIGHT_HTTP_LC_HEADER_LEN - 1);
                	
                	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
                	dprintf(trace_file_fd, "i=%d\n\n", i);
                	
                	break;
            	}
            	
            	// 下划线处理
				if (ch == '_') 
				{
                    hash = night_hash(hash, ch);
                    r->lowcase_header[i++] = ch;
                    i &= (NIGHT_HTTP_LC_HEADER_LEN - 1);
                    
                	break;
            	}
            	
            	// 遇到 :，header 名结束，进入值前的空格状态。
				if (ch == ':') 
				{
					dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
					dprintf(trace_file_fd, "if (ch == ':') \n\n");
					
                	r->header_name_end = p;
                	state = sw_space_before_value;
                	
                	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
                	dprintf(trace_file_fd, "state = sw_space_before_value;\n\n");
                	
                	break;
            	}
            	
				if (ch == CR) 
				{
                	r->header_name_end = p;
                	r->header_start = p;
                	r->header_end = p;
                	state = sw_almost_done;
                	break;
            	}

            	if (ch == LF) 
            	{
                	r->header_name_end = p;
                	r->header_start = p;
                	r->header_end = p;
                	goto done;
            	}
            
            	break;
            	
			// space* before header value
        	case sw_space_before_value:
        		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        		dprintf(trace_file_fd, "case sw_space_before_value:\n\n");
        		
        		switch (ch) 
        		{
        			// 忽略空格
					case ' ':
						dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
						dprintf(trace_file_fd, "case ' ':\n\n");
						
                		break;
                		
					default:
						dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
						dprintf(trace_file_fd, "default:\n\n");
						
                		r->header_start = p;
                		state = sw_value;
                		
                		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
                		dprintf(trace_file_fd, "state = sw_value;\n\n");
                		
                		break;	
        		}
        		break;
        		
			// header value
        	case sw_value:
        		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        		dprintf(trace_file_fd, "case sw_value:\n\n");
        		
        		switch (ch) 
        		{
					case ' ':
						dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
						dprintf(trace_file_fd, "case ' ':\n\n");
						
                		r->header_end = p;
                		state = sw_space_after_value;
                		
                		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
                		dprintf(trace_file_fd, "state = sw_space_after_value;\n\n");
                		
                		break;
                		
            		case CR:
            			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
            			dprintf(trace_file_fd, "case CR:\n\n");
            			
                		r->header_end = p;
                		state = sw_almost_done;
                		
                		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
                		dprintf(trace_file_fd, "state = sw_almost_done;\n\n" );
                		
                		break;
                		
            		case LF:
            			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
            			dprintf(trace_file_fd, "case LF:\n\n");
            			
                		r->header_end = p;
                		
                		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
                		dprintf(trace_file_fd, "goto done;\n\n");
                		
                		goto done;	
        		}
        		break;
        		
			// space before end of header line
        	case sw_space_after_value:
        		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        		dprintf(trace_file_fd, "case sw_space_after_value:\n\n");
        		
				switch (ch) 
				{
            		case ' ':
                		break;
            		case CR:
                		state = sw_almost_done;
                		break;
            		case LF:
                		goto done;
                		
            		default:
                		state = sw_value;
                		break;
            	}
        		break;
        		
			// end of header line
        	case sw_almost_done:
        		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        		dprintf(trace_file_fd, "case sw_almost_done:\n\n");
        	
            	switch (ch) 
            	{
            		case LF:
            			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
            			dprintf(trace_file_fd, "case LF:\ngoto done;\n\n");
            		
                		goto done;
                	
            		case CR:
                		break;
                	
            		default:
                		return NIGHT_HTTP_PARSE_INVALID_HEADER;
            	}
            	break;
            	
			// end of header
        	case sw_header_almost_done:
        		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
        		dprintf(trace_file_fd, "case sw_header_almost_done:\n\n");
        		
            	switch (ch) 
            	{
            		case LF:
            			dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
            			dprintf(trace_file_fd, "case LF:\ngoto header_done;\n\n");
            			
                		goto header_done;
                		
            		default:
                		return NIGHT_HTTP_PARSE_INVALID_HEADER;
            	}		
		}
	}

// 完成一个 header 行
done:
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
	dprintf(trace_file_fd, "done:\n\n");

    b->pos = p + 1;
    r->state = sw_start;
    
    r->header_hash = hash;
    r->lowcase_index = i;

    return NIGHT_OK;

// header 部分结束（空行）
header_done:
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__ );
	dprintf(trace_file_fd, "header_done:\n\n");

    b->pos = p + 1;
    r->state = sw_start;

    return NIGHT_HTTP_PARSE_HEADER_DONE;
}

