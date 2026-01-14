#include "night_core.h"
#include "night.h"
#include "night_times.h"

#define Master_trace 	"trace/Master_trace"
#define CWD_LEN 		(1024)
#define CONF_FILE		"night.conf"

int				trace_file_fd;

int 			night_pid;
int 			night_parent;
night_cycle_t 	*night_cycle;

char	**night_os_argv;
int		night_argc;
char	**night_argv;
char	**night_os_environ;

char 	cwd[CWD_LEN];
char 	conf_prefix[CWD_LEN];
char	conf_file[CWD_LEN];

int		night_dump_config;

int
main(int argc, char *const *argv)
{
	int						rc;
	night_cycle_t      		*cycle, init_cycle;
	
	trace_file_fd = open(Master_trace,O_CREAT|O_TRUNC|O_RDWR,0666);
	
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "main\n\n");
    
	// 命令行参数解析
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"命令行参数解析\n" "night_get_options(argc, argv)\n\n");
	
	rc = night_get_options(argc, argv);
	if (rc != NIGHT_OK) 
	{
        dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(trace_file_fd,"night_get_options(argc, argv) failed\n" "function main:\treturn 1\n\n");
        
        return 1;
    }
	
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"命令行参数解析完成\n" "night_get_options(argc, argv) completed\n\n");
	
	// 时间系统初始化
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"时间系统初始化\n" "night_time_init()\n\n");
	
	night_time_init();
    
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"时间系统初始化完成\n" "night_time_init() completed\n\n");
	
	// 获取当前进程 ID
    night_pid = getpid();
    
    // 获取父进程 ID
    night_parent = getppid();
    
    dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"night_pid=%d\n" "night_parent=%d\n\n", night_pid, night_parent);
    
	// cycle 是 Nginx 架构中最核心的数据结构之一，它是整个 Nginx 服务器的运行时上下文和生命线
    // 本质：cycle 代表 Nginx 的一个完整"运行周期"，包含了服务器运行时所需的所有核心数据
	// 生命周期：从 ngx_init_cycle() 创建开始，直到 Nginx 退出时销毁
	// init_cycle 是临时周期对象,作为初始化阶段的临时载体，用于在真正运行时的 cycle 对象创建之前，承载所有必需的基础资源和配置信息。
	// 为什么需要两个 cycle 对象？
	// init_cycle: 临时的、简化的周期对象，只包含初始化必需的最少信息
	// cycle: 最终的、完整的运行时周期对象，包含所有模块、配置、连接等完整信息
	// 这种设计解决了鸡生蛋蛋生鸡的问题：要创建完整的 cycle 需要很多基础资源，但这些资源本身又需要一个 cycle 来管理。
	
	// init_cycle 初始化
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"init_cycle 初始化\n\n");
	
	night_memzero(&init_cycle, sizeof(night_cycle_t));
	night_cycle = &init_cycle;
	
	// 内存池创建
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"内存池创建\n" "night_create_pool(1024)\n\n");
	
    init_cycle.pool = night_create_pool(1024);
    if (init_cycle.pool == NULL) 
    {
		dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd,"function main:\t" "return 1\n\n");
    	
        return 1;
    }
    
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"内存池创建完成\n" "night_create_pool(1024) completed\n\n");
	
	// 保存命令行参数
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"保存命令行参数\n" "night_save_argv(&init_cycle, argc, argv)\n\n");
	
    if (night_save_argv(&init_cycle, argc, argv) != NIGHT_OK) 
    {
		dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd,"function main:\t" "return 1\n\n");
    	
        return 1;
    }
    
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"保存命令行参数完成\n" "night_save_argv(&init_cycle, argc, argv) completed\n\n");
	
	// 处理进程选项
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"处理进程选项\n" "night_process_options\n\n");
	
    if (night_process_options(&init_cycle) != NIGHT_OK) 
    {
    	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd,"night_process_options failed\n");
    	dprintf(trace_file_fd,"function main:\t" "return 1\n\n");
    	
        return 1;
    }
    
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"处理进程选项完成\n" "night_process_options completed\n\n");
	
	// 系统初始化
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"系统初始化\n" "night_os_init\n\n");
	
    if (night_os_init() != NIGHT_OK) 
    {
		dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd,"night_os_init failed\n");
    	dprintf(trace_file_fd,"function main:\t" "return 1\n\n");
    
        return 1;
    }
    
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"系统初始化完成\n" "night_os_init completed\n\n");
	
	// Slab 内存分配器初始化
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"Slab 内存分配器初始化\n" "night_slab_sizes_init()\n\n");
	
    night_slab_sizes_init();
    
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"Slab 内存分配器初始化 完成\n" "night_slab_sizes_init() completed\n\n");
	
	// 预初始化所有 nginx 模块
	// 为每个模块分配索引和上下文
	// 设置模块的初始化顺序
	// 预初始化失败时返回 1 退出
	
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "预初始化所有模块\n" "night_preinit_modules\n\n");
	
    if (night_preinit_modules() != NIGHT_OK) {
    	
   		dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    	dprintf(trace_file_fd,"function main:\t" "return 1\n\n");
    
        return 1;
    }
    
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "预初始化所有模块 完成\n" "night_preinit_modules completed\n\n");
	
	// night_init_cycle(): 核心函数，创建完整的运行时周期对象
	// 解析配置文件
	// 初始化所有模块
	// 创建监听套接字
	// 设置各种运行时数据结构
	// 创建失败时返回 1 退出

	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"创建完整的运行时周期对象\n" "night_init_cycle\n\n");
	
    cycle = night_init_cycle(&init_cycle);
    if (cycle == NULL) 
    {
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "night_init_cycle failed\n");
		dprintf(trace_file_fd,"function main:\t" "return 1\n\n");
		
        return 1;
    }
	
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"创建完整的运行时周期对象 完成\n" "night_init_cycle completed\n\n");
	
	
    
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function main:\t" "return 0\n\n");
    
	return 0;
}

// 主要作用是 根据命令行参数和编译时配置，对 ngx_cycle_t 结构体中的路径、配置文件、日志文件等关键选项进行初始化和标准化处理
static int
night_process_options(night_cycle_t *cycle)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function:\t" "night_process_options\n\n");
	
	size_t 	len;
	char	*p;
	
	// 获取当前目录作为 prefix
	p = getcwd(cwd, CWD_LEN);
	if (p == NULL)
	{
		dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd,"getcwd failed\n");
		dprintf(trace_file_fd,"function night_process_options:\t" "return NIGHT_ERROR\n\n");
		
		return NIGHT_ERROR;
	}
	
	strcat(cwd, "/");
	
	len = strlen(cwd);
	cycle->prefix.len = len;
	cycle->prefix.data = cwd;
	
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"cycle->prefix.data=%s\n\n", cycle->prefix.data);
	
	memcpy(conf_prefix, cwd, len);
	strcat(conf_prefix, "conf/");
	
	cycle->conf_prefix.len = strlen(conf_prefix);
	cycle->conf_prefix.data = conf_prefix;
	
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"cycle->conf_prefix.data=%s\n\n", cycle->conf_prefix.data);
	
	// 配置文件路径
	memcpy(conf_file, conf_prefix, strlen(conf_prefix));
	strcat(conf_file, CONF_FILE);
	
	cycle->conf_file.len = strlen(conf_file);
	cycle->conf_file.data = conf_file;
		    
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"cycle->conf_file.data=%s\n\n", cycle->conf_file.data);
	
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function night_process_options:\t" "return NIGHT_OK\n\n");
	
	return NIGHT_OK;
}

// 作用是 保存命令行参数（argv）和环境变量（environ），
// 以便 Nginx 在运行时可以安全地访问和修改自己的命令行参数（例如，在进程标题中显示状态信息），
// 而不会破坏原始的 argv 内存布局。
static int
night_save_argv(night_cycle_t *cycle, int argc, char *const *argv)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function:\t" "night_save_argv\n\n");
	
    size_t     	len;
    int  		i;

    night_os_argv = (char **) argv;
    night_argc = argc;

    night_argv = malloc((argc + 1) * sizeof(char *));
    if (night_argv == NULL) 
    {
		dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd,"function night_save_argv:\t" "return NIGHT_ERROR\n\n");
	
        return NIGHT_ERROR;
    }

    for (i = 0; i < argc; i++) 
    {
        len = strlen(argv[i]) + 1;

        night_argv[i] = malloc(len);
        if (night_argv[i] == NULL) 
        {
			dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
			dprintf(trace_file_fd,"function night_save_argv:\t" "return NIGHT_ERROR\n\n");
	
        	return NIGHT_ERROR;
        }

        night_cpystrn((char *) night_argv[i], (char *) argv[i], len);
        
		dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
        dprintf(trace_file_fd,"night_argv[i]=%s\n\n", night_argv[i]);
    }

    night_argv[i] = NULL;

    night_os_environ = environ;

	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function night_save_argv:\t" "return NIGHT_OK\n\n");
	
    return NIGHT_OK;
}

int
night_get_options(int argc, char *const *argv)
{
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd,"function:\t" "night_get_options\n\n");
	
	int					i;
	char*				p;
	
	// 开始主循环，遍历所有命令行参数
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"argc=%d\n\n", argc);
    
	for (i = 1; i < argc; i++) 
	{
		dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
		dprintf(trace_file_fd, "循环，遍历所有命令行参数\ni=%d\n\n", i);
		
		// 将当前参数转换为 char* 类型
        p = (char *) argv[i];
	}
	
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function ngx_get_options:\t" "return NIGHT_OK\n\n");
	
	return NIGHT_OK;
}
