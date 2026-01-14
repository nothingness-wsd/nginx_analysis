# 1. main 函数定义

`./nginx-1.24.0/src/core/nginx.c`中的 main 函数 如下：
```c
int ngx_cdecl
main(int argc, char *const *argv)
{
    ngx_buf_t        *b;
    ngx_log_t        *log;
    ngx_uint_t        i;
    ngx_cycle_t      *cycle, init_cycle;
    ngx_conf_dump_t  *cd;
    ngx_core_conf_t  *ccf;

    ngx_debug_init();

    if (ngx_strerror_init() != NGX_OK) {
        return 1;
    }

    if (ngx_get_options(argc, argv) != NGX_OK) {
        return 1;
    }

    if (ngx_show_version) {
        ngx_show_version_info();

        if (!ngx_test_config) {
            return 0;
        }
    }

    /* TODO */ ngx_max_sockets = -1;

    ngx_time_init();

#if (NGX_PCRE)
    ngx_regex_init();
#endif

    ngx_pid = ngx_getpid();
    ngx_parent = ngx_getppid();

    log = ngx_log_init(ngx_prefix, ngx_error_log);
    if (log == NULL) {
        return 1;
    }

    /* STUB */
#if (NGX_OPENSSL)
    ngx_ssl_init(log);
#endif

    /*
     * init_cycle->log is required for signal handlers and
     * ngx_process_options()
     */

    ngx_memzero(&init_cycle, sizeof(ngx_cycle_t));
    init_cycle.log = log;
    ngx_cycle = &init_cycle;

    init_cycle.pool = ngx_create_pool(1024, log);
    if (init_cycle.pool == NULL) {
        return 1;
    }

    if (ngx_save_argv(&init_cycle, argc, argv) != NGX_OK) {
        return 1;
    }

    if (ngx_process_options(&init_cycle) != NGX_OK) {
        return 1;
    }

    if (ngx_os_init(log) != NGX_OK) {
        return 1;
    }

    /*
     * ngx_crc32_table_init() requires ngx_cacheline_size set in ngx_os_init()
     */

    if (ngx_crc32_table_init() != NGX_OK) {
        return 1;
    }

    /*
     * ngx_slab_sizes_init() requires ngx_pagesize set in ngx_os_init()
     */

    ngx_slab_sizes_init();

    if (ngx_add_inherited_sockets(&init_cycle) != NGX_OK) {
        return 1;
    }

    if (ngx_preinit_modules() != NGX_OK) {
        return 1;
    }

    cycle = ngx_init_cycle(&init_cycle);
    if (cycle == NULL) {
        if (ngx_test_config) {
            ngx_log_stderr(0, "configuration file %s test failed",
                           init_cycle.conf_file.data);
        }

        return 1;
    }

    if (ngx_test_config) {
        if (!ngx_quiet_mode) {
            ngx_log_stderr(0, "configuration file %s test is successful",
                           cycle->conf_file.data);
        }

        if (ngx_dump_config) {
            cd = cycle->config_dump.elts;

            for (i = 0; i < cycle->config_dump.nelts; i++) {

                ngx_write_stdout("# configuration file ");
                (void) ngx_write_fd(ngx_stdout, cd[i].name.data,
                                    cd[i].name.len);
                ngx_write_stdout(":" NGX_LINEFEED);

                b = cd[i].buffer;

                (void) ngx_write_fd(ngx_stdout, b->pos, b->last - b->pos);
                ngx_write_stdout(NGX_LINEFEED);
            }
        }

        return 0;
    }

    if (ngx_signal) {
        return ngx_signal_process(cycle, ngx_signal);
    }

    ngx_os_status(cycle->log);

    ngx_cycle = cycle;

    ccf = (ngx_core_conf_t *) ngx_get_conf(cycle->conf_ctx, ngx_core_module);

    if (ccf->master && ngx_process == NGX_PROCESS_SINGLE) {
        ngx_process = NGX_PROCESS_MASTER;
    }

#if !(NGX_WIN32)

    if (ngx_init_signals(cycle->log) != NGX_OK) {
        return 1;
    }

    if (!ngx_inherited && ccf->daemon) {
        if (ngx_daemon(cycle->log) != NGX_OK) {
            return 1;
        }

        ngx_daemonized = 1;
    }

    if (ngx_inherited) {
        ngx_daemonized = 1;
    }

#endif

    if (ngx_create_pidfile(&ccf->pid, cycle->log) != NGX_OK) {
        return 1;
    }

    if (ngx_log_redirect_stderr(cycle) != NGX_OK) {
        return 1;
    }

    if (log->file->fd != ngx_stderr) {
        if (ngx_close_file(log->file->fd) == NGX_FILE_ERROR) {
            ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_errno,
                          ngx_close_file_n " built-in log failed");
        }
    }

    ngx_use_stderr = 0;

    if (ngx_process == NGX_PROCESS_SINGLE) {
        ngx_single_process_cycle(cycle);

    } else {
        ngx_master_process_cycle(cycle);
    }

    return 0;
}

```

---

# 2.main 函数 逻辑流程

```c
Nginx main() 函数逻辑流程
│
├── 1. 入口与变量声明
│   ├── int ngx_cdecl main(int argc, char *const *argv)
│   └── 声明局部变量（log, cycle, ccf, b 等）
│
├── 2. 命令行与版本处理
│   ├── ngx_debug_init()
│   ├── ngx_strerror_init()
│   ├── ngx_get_options(argc, argv) → 解析 -c, -t, -s, -v, -p 等
│   └── if (ngx_show_version)
│       ├── ngx_show_version_info()
│       └── if (!ngx_test_config) → return 0
│
├── 3. 基础系统初始化
│   ├── ngx_time_init()
│   ├── #if (NGX_PCRE) → ngx_regex_init()
│   ├── ngx_pid = ngx_getpid(); ngx_parent = ngx_getppid()
│   ├── ngx_log_init() → 初始化 error_log
│   └── #if (NGX_OPENSSL) → ngx_ssl_init(log)
│
├── 4. 构建 init_cycle（临时配置周期）
│   ├── ngx_memzero(&init_cycle)
│   ├── init_cycle.log = log
│   ├── ngx_cycle = &init_cycle（临时全局指向）
│   ├── ngx_create_pool(1024, log)
│   ├── ngx_save_argv() → 保存命令行参数
│   ├── ngx_process_options() → 处理 prefix/conf 路径
│   └── ngx_os_init(log) → 获取 pagesize, max_fds 等
│
├── 5. 依赖 OS 的初始化
│   ├── ngx_crc32_table_init()
│   ├── ngx_slab_sizes_init()
│   ├── ngx_add_inherited_sockets() → 热升级继承监听 fd
│   └── ngx_preinit_modules() → 为各模块预分配配置空间
│
├── 6. 加载并验证配置
│   └── cycle = ngx_init_cycle(&init_cycle)
│       ├── 解析 nginx.conf
│       ├── 调用各模块配置 handler
│       ├── 打开监听套接字
│       └── 若失败 → 报错并 return 1
│
├── 7. 特殊模式处理（提前退出）
│   ├── if (ngx_test_config)
│   │   ├── 打印 "test is successful"
│   │   ├── if (ngx_dump_config) → dump 所有 include 配置
│   │   └── return 0
│   │
│   └── if (ngx_signal)
│       └── ngx_signal_process(cycle, ngx_signal) → 发信号后 return
│
├── 8. 正式启动准备（Unix only）
│   ├── ngx_os_status() → 打印系统信息（debug 用）
│   ├── ngx_cycle = cycle（切换全局 cycle 指针）
│   ├── 获取 ccf = core config
│   ├── if (ccf->master) → ngx_process = NGX_PROCESS_MASTER
│   │
│   └── #if !(NGX_WIN32)
│       ├── ngx_init_signals() → 注册 SIGTERM, SIGHUP 等
│       ├── if (!inherited && daemon) → ngx_daemon() → 后台化
│       └── if (inherited) → ngx_daemonized = 1
│
├── 9. 运行时资源设置
│   ├── ngx_create_pidfile(&ccf->pid) → 写入 master PID
│   ├── ngx_log_redirect_stderr(cycle) → stderr → error_log
│   ├── 关闭早期日志 fd（若非 stderr）
│   └── ngx_use_stderr = 0
│
└── 10. 进入主事件循环
    ├── if (NGX_PROCESS_SINGLE)
    │   └── ngx_single_process_cycle(cycle)
    │       └── 单进程：直接运行 worker 逻辑
    │
    └── else
        └── ngx_master_process_cycle(cycle)
            └── master 进程：管理 worker 生命周期、信号、热升级等
```

---
# 3.代码分析 & copy
```
我将采用一种愚蠢但最扎实的办法来理解 源代码，

那就是自己创建一个项目，然后 copy nginx 源码，
理解的就 copy 过来，还不理解的就暂时跳过。

对于部分代码会有"为什么一定要这样写，换一种写法，那样写可不可以？"的疑问

那就按照我自己的想法改写，

然后通过运行效果的对比，bug 原因的查找 分析和修复来理解原本不能理解的部分

对于在 运行效果的对比，bug 原因的查找 分析和修复
中也没显现出来的问题，就暂时先放过它们。

对于没有在实践中凸显出的问题，就算想要通过书面的理论来理解也只能是模糊的，似是而非的理解，
在实践中凸显出的问题，然后努力去设法解决，如此才有最坚实的理解
```

```
对于“不要重复造轮子”这句话，
可是如果没有自己真正造过这样一个轮子，
如何确定自己真正知道它是怎么被造出来的，
如何敢确定自己有造出这样的轮子的能力？
```

---

创建 一个night项目来 copy nginx

首先创建 `night/src/core/night.c`，来 copy `nginx.c`

---

## 1.函数签名
```c
int ngx_cdecl
main(int argc, char *const *argv)
```
- [ngx_cdecl](https://blog.csdn.net/weixin_41812346/article/details/145421815?spm=1011.2415.3001.5331)

同样 在 `night.c`中定义 main 函数
```c
int
main(int argc, char *const *argv)
{

}
```
我需要加入输出来追踪运行情况

于是

在 `nginx.c`中加入全局变量
```c
int trace_file_fd;
```

一个文件描述符

main 函数中加入

```c
trace_file_fd = open(Master_trace,O_CREAT|O_TRUNC|O_RDWR,0666);
```
打开一个文件作为 输出文件,文件描述符保存在全局变量 `trace_file_fd`中以供后续使用

在 `ngx_core.h`中加入

```c
#define Master_trace 		"trace/Master_trace"
extern int trace_file_fd;
```
定义了文件的相对路径

声明 trace_file_fd 为外部全局变量，这样在其他源文件中通过 引入了 `ngx_core.h` 而包含了 `extern int trace_file_fd;`声明，这样才能在其他源文件中使用 trace_file_fd 全局变量而不会报未声明的错误

然后加入
```c
	dprintf(trace_file_fd,"FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
    dprintf(trace_file_fd,"function:\t" "%s\n\n", __func__);
```
这样的代码来追踪运行的情况

---

同样在我的 `night.c` 中也要加入这样的输出

---

接下来的问题是编译我的 night 项目源码

同样使用 makefile 来编译，它就在`night/Makefile` 中
```
# 默认目标
all: build
```
这是设置默认目标（第一个目标），让它依赖于 build，
<br>
当执行 make 命令但不指定具体目标时它就执行这个默认目标（第一个目标）

也就是执行 
```
sudo make
```
相当于执行
```
sudo make all
```
相当于执行
```
sudo make build
```

```
# 编译目标
build: \
	objs/src/core/night.o \
	objs/src/core/night_times.o \
	objs/src/os/unix/night_time.o
	
	gcc -o night \
	objs/src/core/night.o \
	objs/src/core/night_times.o
```
build 目标是在执行 编译的链接，空行前是执行时所要检查的依赖，空行后是要执行的命令

> 要注意每一行前面的是 tab键，不是空格

```
objs/src/core/night.o: \
	src/core/night.c \
	src/core/night.h \
	src/core/night_core.h
	
	gcc -c $(INCLUDE) \
	-o objs/src/core/night.o \
	src/core/night.c
```
这样的是在编译某个 `.c`源文件，
<br>
`gcc -c`只编译源文件，但不进行链接（生成目标文件 .o 文件） 

`$(INCLUDE)` 替换为开头定义的 `INCLUDE`变量
```
# 头文件目录
INCLUDE = -I./src/core -I./src/event -I./src/event/modules -I./src/http -I./src/http/modules -I./src/os/unix
```
`-I` 指定头文件搜索目录,
<br>
当自己定义的头文件与 `.c`文件不在同一个目录下，
编译器找不到 头文件
<br>
需要 `-I` 指定头文件搜索目录，然后编译器会去 `-I` 指定头文件搜索目录下查找需要的头文件

```
# 清理目标
clean:
	rm -f night && \
	find objs/src/ -type f -delete
```
`make clean` 将执行这个目标
```
rm -f night
```
会移除之前编译生成的可执行文件
```
find objs/src/ -type f -delete
```
查找 `objs/src/` 目录下的文件 
<br>
`-type f`指定 文件类型为普通文件，而不是目录
<br>
`-delete` 对找到的文件执行删除
<br>
也就是删除`objs/src/` 目录下的所有文件但保留目录 
<br>
这是在删除之前 `.c` 文件编译生成的 `.o` 文件
<br>
每一次我都会先执行 `make clean`清理之前编译生成的所有文件，再 `make` 重新编译

---

```
# 声明伪目标
.PHONY: all build clean
```
在 Makefile 中，`.PHONY: all build clean` 的作用是**声明 `all`、`build`、`clean` 这些目标为“伪目标”（phony targets）**，即它们不代表实际的文件名，而是代表一组操作或命令。

---

为什么需要 `.PHONY`？

Make 的默认行为是：  
> **如果目标名称与磁盘上的某个文件同名，且该文件存在且比依赖项新，Make 就认为目标已是最新的，不会执行其命令。**

这在处理**真实文件目标**时非常有用，但对**逻辑性目标**（如 `clean`、`all`）会造成问题。

---

举个例子说明问题

假设你有如下 Makefile：

```makefile
clean:
	rm -f *.o program
```

正常情况下，运行 `make clean` 会删除编译产物。

但如果**不小心创建了一个名为 `clean` 的文件**（比如 `touch clean`），那么下次运行 `make clean` 时：

- Make 发现文件 `clean` 存在；
- 它没有依赖项；
- 所以 Make 认为 “`clean` 已经是最新的”，**跳过执行命令**！
- 结果：`clean` 命令根本没运行！

这就是问题所在。

---

使用 `.PHONY` 解决问题

```makefile
.PHONY: clean

clean:
	rm -f *.o program
```

加上 `.PHONY: clean` 后，Make **明确知道 `clean` 不是一个文件目标，而是一个动作**，因此：

- 无论是否存在名为 `clean` 的文件，
- 每次运行 `make clean` 都会**无条件执行命令**。

---

### 常见的伪目标包括：

| 伪目标 | 用途 |
|--------|------|
| `all`      | 默认构建所有内容（通常是第一个目标） |
| `clean`    | 清理编译生成的文件 |
| `install`  | 安装程序到系统目录 |
| `test`     | 运行测试 |
| `build`    | 显式触发构建（有时与 `all` 同义） |

---

### 💡 最佳实践

- **所有不生成同名文件的目标都应声明为 `.PHONY`**。
- 即使你确定不会出现同名文件，也建议加上 `.PHONY` —— 这是规范、安全、可读性强的做法。
- 如果不加，可能在某些特殊场景（如自动生成文件、CI 环境）中导致难以排查的问题。

---

### ✅ 总结

```makefile
.PHONY: all build clean
```

这行代码告诉 Make：

> “`all`、`build`、`clean` 不是文件，而是命令标签，请每次都执行它们对应的命令，不要因为磁盘上有同名文件就跳过！”

这是编写健壮、可靠 Makefile 的**必备习惯**。

---

