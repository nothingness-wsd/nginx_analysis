
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


#if (NGX_SETPROCTITLE_USES_ENV)

/*
 * To change the process title in Linux and Solaris we have to set argv[1]
 * to NULL and to copy the title to the same place where the argv[0] points to.
 * However, argv[0] may be too small to hold a new title.  Fortunately, Linux
 * and Solaris store argv[] and environ[] one after another.  So we should
 * ensure that is the continuous memory and then we allocate the new memory
 * for environ[] and copy it.  After this we could use the memory starting
 * from argv[0] for our process title.
 *
 * The Solaris's standard /bin/ps does not show the changed process title.
 * You have to use "/usr/ucb/ps -w" instead.  Besides, the UCB ps does not
 * show a new title if its length less than the origin command line length.
 * To avoid it we append to a new title the origin command line in the
 * parenthesis.
 */

extern char **environ;

static char *ngx_os_argv_last;

// 其主要作用是为后续通过修改进程标题（process title）提供支持。
// 在 Unix-like 系统中，进程的命令行参数（argv）和环境变量（environ）通常在内存中是连续存放的，
// 并且内核会根据 argv[0] 的起始地址和长度来显示 ps 或 top 等命令中的进程名。
// Nginx 为了能够动态修改显示的进程名（比如在 master/worker 进程中显示 “nginx: worker process”），
// 需要确保 argv 和 environ 所占的内存是可以安全覆盖的，并且不会影响其他内存区域。
// 然而，很多系统并不允许直接修改 environ 指向的原始环境变量内存（可能位于只读段或与栈共用），
// 因此 Nginx 在初始化阶段会将 environ 数组中那些与 argv 内存区域连续的环境变量复制到一个新分配的、可写的堆内存中，从而为后续 setproctitle（或等效操作）扫清障碍。
ngx_int_t
ngx_init_setproctitle(ngx_log_t *log)
{
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function:\t" "ngx_init_setproctitle\n\n");
	
    u_char      *p;
    size_t       size;
    ngx_uint_t   i;

	// 初始化 size 为 0，用于累加所有环境变量字符串的总长度（包括结尾的 '\0'）
    size = 0;

	// 遍历全局变量 environ
	// 对每个环境变量字符串，计算其长度（ngx_strlen）并加 1（用于 '\0' 结束符），累加到 size。
	// 目的：计算复制所有环境变量所需的最大内存总量。
    for (i = 0; environ[i]; i++) {
        size += ngx_strlen(environ[i]) + 1;
    }
    
	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "size=%ld\n\n", size);
	
	// 分配 size 字节的内存
    p = ngx_alloc(size, log);
    if (p == NULL) {
        return NGX_ERROR;
    }

	// ngx_os_argv_last 是一个全局指针（u_char *），用于标记 argv 区域的结束位置。
	// 初始将其设为 argv[0] 的起始地址
    ngx_os_argv_last = ngx_os_argv[0];

	// 遍历 argv 数组
    for (i = 0; ngx_os_argv[i]; i++) {
        if (ngx_os_argv_last == ngx_os_argv[i]) {
            ngx_os_argv_last = ngx_os_argv[i] + ngx_strlen(ngx_os_argv[i]) + 1;
        }
    }

	// 再次遍历 environ 数组
	// 检查当前环境变量 environ[i] 的地址是否正好等于 ngx_os_argv_last。
	// 如果是，说明该环境变量紧跟在 argv 区域之后，属于“连续内存区域”的一部分。
	// 只有这些“紧邻 argv 的环境变量”才需要被复制，因为后续 setproctitle 会覆盖从 argv[0] 开始到 ngx_os_argv_last 的内存。
    for (i = 0; environ[i]; i++) {
        if (ngx_os_argv_last == environ[i]) {

            size = ngx_strlen(environ[i]) + 1;
            ngx_os_argv_last = environ[i] + size;

            ngx_cpystrn(p, (u_char *) environ[i], size);
            environ[i] = (char *) p;
            p += size;
        }
    }

	// 指向整个 argv+environ 连续区域的最后一个字节（通常是 '\0'）
    ngx_os_argv_last--;

	dprintf(trace_file_fd, "FILE=%s:LINE=%d\n" , __FILE__, __LINE__);
	dprintf(trace_file_fd, "function ngx_init_setproctitle:\t" "return NGX_OK\n\n");
	
    return NGX_OK;
}


void
ngx_setproctitle(char *title)
{
    u_char     *p;

#if (NGX_SOLARIS)

    ngx_int_t   i;
    size_t      size;

#endif

    ngx_os_argv[1] = NULL;

    p = ngx_cpystrn((u_char *) ngx_os_argv[0], (u_char *) "nginx: ",
                    ngx_os_argv_last - ngx_os_argv[0]);

    p = ngx_cpystrn(p, (u_char *) title, ngx_os_argv_last - (char *) p);

#if (NGX_SOLARIS)

    size = 0;

    for (i = 0; i < ngx_argc; i++) {
        size += ngx_strlen(ngx_argv[i]) + 1;
    }

    if (size > (size_t) ((char *) p - ngx_os_argv[0])) {

        /*
         * ngx_setproctitle() is too rare operation so we use
         * the non-optimized copies
         */

        p = ngx_cpystrn(p, (u_char *) " (", ngx_os_argv_last - (char *) p);

        for (i = 0; i < ngx_argc; i++) {
            p = ngx_cpystrn(p, (u_char *) ngx_argv[i],
                            ngx_os_argv_last - (char *) p);
            p = ngx_cpystrn(p, (u_char *) " ", ngx_os_argv_last - (char *) p);
        }

        if (*(p - 1) == ' ') {
            *(p - 1) = ')';
        }
    }

#endif

    if (ngx_os_argv_last - (char *) p) {
        ngx_memset(p, NGX_SETPROCTITLE_PAD, ngx_os_argv_last - (char *) p);
    }

    ngx_log_debug1(NGX_LOG_DEBUG_CORE, ngx_cycle->log, 0,
                   "setproctitle: \"%s\"", ngx_os_argv[0]);
}

#endif /* NGX_SETPROCTITLE_USES_ENV */
