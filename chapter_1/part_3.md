继续 main 函数的分析

---

- [源码分析](https://blog.csdn.net/weixin_41812346/article/details/156025283?sharetype=blogdetail&sharerId=156025283&sharerefer=PC&sharesource=weixin_41812346&spm=1011.2480.3001.8118)

```c
ngx_debug_init
```
- [ngx_debug_init](https://blog.csdn.net/weixin_41812346/article/details/145423293?spm=1011.2415.3001.5331)

---

```c
    if (ngx_strerror_init() != NGX_OK) {
        return 1;
    }
```
- [ngx_strerror_init](https://blog.csdn.net/weixin_41812346/article/details/145424631?spm=1011.2415.3001.5331)

`NGX_OK` 一个宏，代表一个函数执行成功,
<br>
不成功就 `return 1` 结束 main 函数

`NGX_OK` 定义在 `src/core/ngx_core.h`
```c
#define  NGX_OK          0
#define  NGX_ERROR      -1
#define  NGX_AGAIN      -2
#define  NGX_BUSY       -3
#define  NGX_DONE       -4
#define  NGX_DECLINED   -5
#define  NGX_ABORT      -6
```
`NGX_ERROR` 则代表失败
<br>
`NGX_AGAIN` 代表数据未准备好，请稍后再试
<br>
`NGX_BUSY`  标识某资源正忙

---

在 C 语言中， 
<br>
函数返回 0 常表示没有错误的状态，
<br>
而正整数的返回值则常表示字节数
<br>
所以其他状态就使用负数来代表

---

同样的在 `night_core.h`中定义
```c
#define  NIGHT_OK				0
#define  NIGHT_ERROR			-1
#define  NIGHT_AGAIN			-2
#define  NIGHT_BUSY				-3
#define  NIGHT_DONE				-4
#define  NIGHT_DECLINED			-5
#define  NIGHT_ABORT			-6
```
`night_core.h` 这个头文件
1. 定义一些用于全局的，各个源文件中都可能用到的宏

2. 声明 `extern`全局变量，
<br>
> 当一个源文件中使用了某个全局变量 a,
<br>
而这个全局变量 a 的定义在另一个源文件中时，
<br>
必须要在使用前声明 `extern int a` ，否则编译这个源文件时会报错
<br>
不知道这个 全局变量是哪里来的。
<br>
声明在这个头文件按中，通过引入这个头文件来 做这件事

3. 集中头文件的引入
```c
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include <sys/stat.h>
#include <errno.h>
#include <stddef.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <pwd.h>
#include <grp.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <sys/utsname.h>
```
```c
#include "night_times.h"
#include "night_palloc.h"
#include "night_rbtree.h"
#include "night_string.h"
#include "night_array.h"
#include "night_file.h"
#include "night_buf.h"
#include "night_conf_file.h"
#include "night_list.h"
#include "night_open_file.h"
#include "night_shm_zone.h"
#include "night_listening.h"
#include "night_queue.h"
#include "night_conf.h"
#include "night_command.h"
#include "night_module.h"
#include "night_files.h"
#include "night_cycle.h"
#include "night_modules.h"
#include "night_string.h"
#include "night_slab.h"
#include "night_core_conf.h"
#include "night_core_module_ctx.h"
#include "night_hash_keys_arrays.h"
#include "night_hash.h"
#include "night_variable_value.h"
#include "night_event.h"
#include "night_http.h"

#include "night_os_config.h"
```
将头文件的引入都集中到这里，然后每个`.c`文件的开头只需要引入 `night_core.h`这一个就行了，不然每个`.c`文件的开头都要写一堆 `#include 头文件`

---

#include <header.h> 写法：
<br>
- 搜索路径：
<br>
仅在系统预定义的目录中查找（如 /usr/include、编译器内置路径等）。
<br>
<br>
- 典型用途：
<br>
标准库头文件（如 <stdio.h>、<stdlib.h>）或通过 -I 指定的第三方库头文件。
不会在当前源文件所在目录中查找。

---

#include "header.h"（双引号） 写法：

- 搜索路径：
首先在当前源文件所在的目录中查找；
如果没找到，再退化为像 #include <header.h> 一样，在系统目录中查找。

- 典型用途：项目内部的自定义头文件（如 "my_utils.h"、"config.h"）。

---

项目自己的头文件始终用 ""，系统/第三方库用 <> —— 这是社区约定，提高可读性

---

```
对于自定义的头文件部分需要注意书写的顺序，

如果 B中使用了A中定义的内容,
那么A要写在前面，B要写在后面，
否者编译会报 不知道某个标识符的错

这个问题在书写时要考虑，
但如果一时不能想清楚就等报错了再调整，
如果每次写的时候都花大量时间去理清楚先后包含问题再编译，
追求编译不报错，是没有必要的。

让编译报错后再修改比追求编译前就完美无暇更加现实和节约时间
```
---

```c
#ifndef _NIGHT_CORE_H_
#define _NIGHT_CORE_H_

#endif /* _NIGHT_CORE_H_ */
```
```
这是每一个头文件的基本框架
用这样的条件编译来避免头文件重复包含

当一个头文件a.h中需要 c.h,在 a.h 中引入了 c.h，
而同样的 b.h 中也引入 c.h,
那么在 同时引入 a.h 和 b.h 时， c.h出现了 2 次

或者 a.h 中包含了 b.h, b.h中又包含了 c.h,
在引入 a.h 时不知道已经包含了 c.h
所以同时引入了 a.h 和 c.h，这样 c.h 出现了 2 次

这就需要上面的条件编译来确保不会重复包含

如果没有定义宏 _NIGHT_CORE_H_，那就表示之前没有包含这个头文件，
然后定义这个宏，引入其后的内容，
如果其后再一次出现这个头文件，第一行的条件编译就不成立
其后内容就不会被引入
```

---

```c
#define _GNU_SOURCE
```
在 C 语言中，`#define _GNU_SOURCE` 是一个**特性测试宏（Feature Test Macro）**，它的作用是**启用 GNU/Linux 系统（特别是 glibc）中的一系列非标准但非常有用的扩展功能**。

---

### 🔍 核心作用

当你在包含任何系统头文件（如 `<stdio.h>`、`<unistd.h>` 等）**之前**定义 `_GNU_SOURCE`：

```c
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// ...
```

你告诉 **glibc（GNU C Library）**：“请暴露所有 GNU/Linux 特有的、POSIX 或 ISO C 标准之外的函数、宏和定义”。

---

### ✅ 启用哪些功能？（常见例子）

| 功能 | 说明 |
|------|------|
| **`strdup()`**, **`strndup()`** | 复制字符串（ISO C 没有，POSIX 有，但需 `_GNU_SOURCE` 或 `_POSIX_C_SOURCE`） |
| **`getline()`** | 从流中读取整行（比 `fgets` 更方便） |
| **`asprintf()`**, **`vasprintf()`** | 动态分配内存的 `printf`（返回格式化后的字符串指针） |
| **`daemon()`** | 将进程转为守护进程 |
| **`get_current_dir_name()`** | 获取当前工作目录（比 `getcwd(NULL, 0)` 更直接） |
| **`pthread_setname_np()`** | 设置线程名（用于调试） |
| **`O_TMPFILE`** | Linux 特有的安全临时文件标志（`open()` 中使用） |
| **`CPU_SET` 宏族** | CPU 亲和性操作（`sched_setaffinity` 相关） |
| **`epoll_create1()`**, **`accept4()`** | 带 flags 的现代系统调用 |

> 💡 很多这些函数以 `_np`（non-portable）结尾，明确表示“非可移植”。

---

### ⚠️ 重要注意事项

#### 1. **必须在包含任何头文件前定义**
```c
// ✅ 正确
#define _GNU_SOURCE
#include <stdio.h>

// ❌ 错误：可能无效
#include <stdio.h>
#define _GNU_SOURCE  // 太晚了！
```

#### 2. **降低代码可移植性**
- `_GNU_SOURCE` 是 **Linux/glibc 特有**的。
- 在 macOS、FreeBSD、Windows 等系统上，这些函数可能不存在。
- 如果需要跨平台，应使用条件编译或替代实现。

#### 3. **与其他特性宏互斥**
- `_GNU_SOURCE` 会**自动启用**：
  - `_ISOC99_SOURCE`
  - `_POSIX_SOURCE`
  - `_POSIX_C_SOURCE`（设为 200809L）
  - `_XOPEN_SOURCE`（设为 700）
- 所以**不要同时定义** `_POSIX_C_SOURCE` 和 `_GNU_SOURCE`，否则可能冲突。

---

### 🛠 替代方式（推荐用于大型项目）

在编译时通过 `-D` 定义，而不是写死在源码中：

```bash
gcc -D_GNU_SOURCE -o myapp myapp.c
```

优点：
- 源码更干净；
- 可通过构建系统（如 Makefile、CMake）统一控制。

---

### 📚 官方文档参考

- [glibc 手册：Feature Test Macros](https://www.gnu.org/software/libc/manual/html_node/Feature-Test-Macros.html)
- man 手册页中常有说明，例如：
  ```bash
  man getline
  ```
  会显示：
  > Feature Test Macro Requirements for glibc:
  > ```c
  > #define _GNU_SOURCE
  >         /* or */
  > #define _POSIX_C_SOURCE 200809L
  > ```

---

### ✅ 总结

| 项目 | 说明 |
|------|------|
| **作用** | 启用 GNU/Linux（glibc）特有扩展功能 |
| **位置** | 必须在 `#include` 任何系统头文件**之前**定义 |
| **用途** | 使用 `getline`, `asprintf`, `epoll_create1` 等强大但非标准的函数 |
| **代价** | 代码绑定到 GNU/Linux 平台，失去可移植性 |
| **建议** | 小型 Linux 工具可直接用；大型/跨平台项目慎用或封装 |

> 简单说：**`#define _GNU_SOURCE` = “我要用 Linux 最强大的系统功能，不在乎跨平台”**。

---

要使用 `dprintf` 函数就需要 `#define _GNU_SOURCE` 

`dprintf` 是一个 GNU 扩展函数

功能：像 fprintf 一样格式化输出，但直接写入文件描述符（fd），而不是 FILE* 流。

它是 printf + write 的便捷组合。

---

**为什么需要 _GNU_SOURCE？**

dprintf 不是 ISO C 标准函数。
它最初是 GNU 扩展，后来被纳入 POSIX.1-2008。
但在 glibc（GNU C Library） 中，为了兼容旧代码和控制暴露的接口，默认不启用 POSIX.1-2008 及 GNU 扩展，除非你显式启用。

---

由于
```c
    ngx_debug_init();

    if (ngx_strerror_init() != NGX_OK) {
        return 1;
    }
```
这部分代码并没有实际作用所以在 night 的 main 函数中就不需要了

---

接下来是
```c
    if (ngx_get_options(argc, argv) != NGX_OK) {
        return 1;
    }
```
- [命令行参数解析](https://blog.csdn.net/weixin_41812346/article/details/156028607?sharetype=blogdetail&sharerId=156028607&sharerefer=PC&sharesource=weixin_41812346&spm=1011.2480.3001.8118)    

同样在 `night.c` 中定义
```c
int
night_get_options(int argc, char *const *argv)
```
我把它写在了 main 函数之后，所以需要在 `night.h` 中声明，然后在 `night.c` 的开头引入头文件。
<br>
<br>
在函数被调用之前要能够找到它的声明才行，定义可以写在 调用之后。
<br>

---

继续分析 main 函数的代码
```c
    if (ngx_show_version) {
        ngx_show_version_info();

        if (!ngx_test_config) {
            return 0;
        }
    }
```
```
版本信息显示

如果命令行包含 -v 或 -V 参数，显示 nginx 版本信息

ngx_show_version_info(): 打印版本、编译参数等详细信息

如果只是显示版本且不是配置测试模式，直接返回 0 退出程序

这次执行是用
sudo ./nginx
来启动运行的
不走这个条件分支
```

---


