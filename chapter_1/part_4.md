继续 main 函数分析

```c
ngx_time_init();
```
时间系统初始化

- [ngx_time_init](https://blog.csdn.net/weixin_41812346/article/details/156053372?sharetype=blogdetail&sharerId=156053372&sharerefer=PC&sharesource=weixin_41812346&spm=1011.2480.3001.8118)

---

```c
void
ngx_time_init(void)
{
    ngx_cached_err_log_time.len = sizeof("1970/09/28 12:00:00") - 1;
    ngx_cached_http_time.len = sizeof("Mon, 28 Sep 1970 06:00:00 GMT") - 1;
    ngx_cached_http_log_time.len = sizeof("28/Sep/1970:12:00:00 +0600") - 1;
    ngx_cached_http_log_iso8601.len = sizeof("1970-09-28T12:00:00+06:00") - 1;
    ngx_cached_syslog_time.len = sizeof("Sep 28 12:00:00") - 1;

    ngx_cached_time = &cached_time[0];

    ngx_time_update();
}
```

---

同样的创建 `night/src/core/night_times.c`,
`night/src/core/night_times.h`

在 `night_times.c` 中定义 `night_time_init` 函数

对于预设各种时间格式的字符串的长度这一部分，只保留
```c
night_cached_http_time.len = sizeof("Mon, 28 Sep 1970 06:00:00 GMT") - 1;
```
 用于在 HTTP 响应头中快速输出标准时间，避免每次格式化

其他时间格式暂时搁置，等到之后有明确用途后再考虑

`night_cached_time`等全局变量就定义在函数前面

而 `night_time_t` 定义在 `night_times.h`中

```c
typedef struct night_time_s night_time_t;

struct night_time_s
{
    time_t	sec;
    time_t  msec;
    int		gmtoff;
};
```
第一个字段是 秒数，第二个字段是 毫秒数，第三个字段是时区偏移

`night_time_t` 是用 `typedef` 对 `struct night_time_s` 的重定义

这样重定义之后，使用时直接写 `night_time_t` ，相比写 `struct night_time_s` 省去了 `struct`,

如此在形式上更接近像 `int` `char` 这样的基本类型的写法

```
而另一个作用是 typedef 这一行可以分离开写到 night_core.h 的开头去

当有另一个文件 a.h 中需要用到 night_time_t* 类型的指针时，
那么 night_time_t 类型的声明就要写在 a.h 之前，
但如果 night_times.h 中又需要某个 a.h 中的声明时， 那么 a.h 应该写在 night_times.h 之前
这样相互嵌套的关系 可以将 typedef 这一行提出来写到 night_times.h 和 a.h 之前来解决
```

以上就是我对于 为什么要 `typedef` 结构体的思考，

后缀 `_s`,这个 `s` 代表的是 `struct`,

而后缀`_t`,这个 `t` 代表的是 `typedef`。

```
在上述 存在相互嵌套关系时，
如果 a.h 中需要用到的不是 night_time_t* 类型的指针，
而是 night_time_t ，那就需要 night_time_t 的完整定义 也就是 night_time_s{} 部分的定义需要写在 a.h 之前，
那么前面讲的  typedef 这一行提前的方法也就不行了

这就需要解除头文件相互嵌套的关系，
所以如果一个结构体需要在其他头文件中被使用，那么它就单独占据一个头文件，这个头文件中不再定义另一个结构体，而能够写在这个头文件中的 函数声明 要是操作这个结构体 这样功能的函数，否则就分开写到另一个头文件中，

这样 按逻辑进一步细分，分清楚，分写到不同的头文件中，然后 通过 多个 
#include "header.h"
的写法把多个头文件组合起来，用 逻辑细分 + 组合 使用的方式，避免相互嵌套的方式

同时在创建每一个的 .c 文件， .h 文件时，要明确它的逻辑功能，
它是做什么的，把逻辑分清楚，边界分清楚，
以此判断一个函数或定义，声明 到底应该写在哪一个文件中，让这一切逻辑清晰，都有一个明确的说法，而不能模糊的，似是而非的去决定

一个函数也是如此，要明确它的功能，成为一个明确的逻辑单元，组合到一个更大的整体中去。

这样逻辑明确，可读性高，比一味追求性能更重要，
从我看过的《为什么学生不喜欢上学》《认知天性》等关于认知方法的书籍和观点中可以得知 

人脑的运行内存是有限的，

在我曾看过的另一个关于写作方法的书中也讲到 写的文章要层次分明，而每一层就 3，5 个论点，不要太多，这样对于读者才逻辑清晰，更好理解

对于我们人类而言 人脑的运行内存是有限的，
如果不分出层次，只写一个 main 函数，那么这个 main 函数就太长了，人去阅读它，梳理它的逻辑时，就会感到 运行内存 不够用，

而当它分出层次，把一个 main 函数分作几个逻辑单元的组合时，这个 main 函数的长度就降下来了，我们去梳理它的逻辑时就更容易一些，

而每一个逻辑部分又封装到一个个函数中去实现，这样一个个逻辑单元分层次组合的感觉对于梳理清楚一个庞大系统的逻辑是很有帮助的，
这比起追求性能还要重要，
毕竟一个较大的系统性能差一点也能运行，但是如果逻辑梳理不清楚，那么可就代表着要出现 bug。

事物都有 2 面性，这常体现在一些算法中，
有的算法 时间换空间，有的 空间换时间，都有代价，

在《操作系统之哲学原理》中也多次提到 "差不多"的哲学原理，追求极致是有开销的，要付出代价的，越是追求极致 满分，代价越大，远大于合格的代价

总之，逻辑清晰是我第一追求的目标，而我的性格就是喜欢 慢一点，逻辑更清晰一点，所以我是一个反应迟钝的人，但我要追求逻辑清晰

```
---

接下来是：
```c
ngx_cached_time = &cached_time[0];
```
`cached_time` 是一个全局的数组

这个数组的定义就在本文件的顶部

这个数组中的每一个元素都是用来记录时间的

`ngx_cached_time` 全局变量指向当前使用的数组中的某个槽位

---

`cached_time` 这个数组的意义？

`cached_time` 数组是 Nginx 高性能时间处理机制中的核心数据结构，其设计目的是**避免在高并发请求处理中频繁调用系统时间函数**，从而显著提升性能。

### 一、为什么需要 `cached_time`？

在 Web 服务器中，时间信息被广泛使用，例如：

- 生成 HTTP 响应头中的 `Date` 字段；
- 写入 access log / error log 的时间戳；
- 处理缓存过期（如 `Expires`、`Cache-Control`）；
- 限流、超时控制等内部逻辑。

如果每个请求都调用 `gettimeofday()`（或 `clock_gettime()`），会带来以下问题：

- **系统调用开销大**：即使现代 CPU 有 vDSO 优化，频繁调用仍会消耗 CPU 周期；
- **时间精度要求不高**：大多数场景（如日志、HTTP Date）只需秒级精度，不需要每次精确到微秒；
- **可接受轻微延迟**：Nginx 允许时间“略微滞后”，比如每秒更新一次完全足够。

因此，Nginx 采用 **“定时批量更新 + 缓存读取”** 的策略。

---

### 二、`cached_time` 数组的定义（以 Nginx 1.24.0 为例）

在 `src/core/ngx_times.c` 中，通常有如下定义：

```c
static ngx_time_t cached_time[NGX_TIME_SLOTS];
```

其中：

- `NGX_TIME_SLOTS` 默认为 **64**（可通过编译选项调整）；
- `ngx_time_t` 是一个结构体，包含：
  ```c
  typedef struct {
      time_t      sec;          // 秒
      ngx_uint_t  msec;         // 毫秒
      ngx_int_t   gmtoff;       // 时区偏移
  } ngx_time_t;
  ```

此外，还有多个全局变量指向不同格式的时间字符串，例如：

---

### 三、`cached_time` 的工作机制

1. **循环缓冲（Ring Buffer）**  
   `cached_time` 是一个大小为 64 的环形数组。每次更新时间时，Nginx 不覆盖旧数据，而是写入下一个槽位（slot）。

2. **由事件循环定期更新**  
   在 master 或 worker 进程的事件循环中（如 epoll_wait 返回后），Nginx 会检查是否已过去 1 秒（或其他时间间隔）。如果是，则调用 `ngx_time_update()`。

3. **`ngx_time_update()` 做什么？**
   - 调用 `gettimeofday()` 获取当前时间；
   - 计算秒、毫秒、时区等；
   - 将新时间写入 `cached_time[next_slot]`；
   - 更新 `ngx_cached_time = &cached_time[next_slot]`；
   - **同时格式化多种时间字符串**（HTTP、log、ISO8601 等），并复制到各自的全局缓存数组中（如 `ngx_cached_http_time.data` 指向的内容）；
   - `next_slot = (next_slot + 1) % NGX_TIME_SLOTS`，实现循环。

4. **请求处理时直接读取缓存**  
   在处理 HTTP 请求、写日志等操作时，代码直接使用 `ngx_cached_time->sec` 或 `ngx_cached_http_time`，**无需任何系统调用**。

---

### 四、为何使用数组（而非单个变量）？

你可能会问：既然每秒更新一次，为什么不用一个 `ngx_time_t current_time` 变量，而要用 64 个槽位的数组？

原因如下：

#### ✅ 1. **支持多线程/多核下的无锁读取（Lock-Free）**
- 在多 worker 进程模型中，虽然每个进程独立，但某些模块（如共享内存统计）可能涉及跨请求时间比较。
- 更重要的是，在**单个 worker 内部**，可能存在异步操作（如 timer、subrequest）引用了旧的时间值。
- 使用环形缓冲区可以保证：即使时间被更新，之前保存的 `ngx_time_t*` 指针仍然有效（因为旧槽位未被立即覆盖）。

#### ✅ 2. **防止 ABA 问题**
- 如果只用一个变量，当时间更新非常快（如调试时加速时钟），指针值可能“看起来没变”，但实际上内容已变。
- 环形缓冲确保每次更新都写入新地址，指针值变化可作为“版本号”使用。

#### ✅ 3. **便于调试和追踪**
- 保留最近 64 秒的时间快照，有助于诊断时间相关的问题（虽然生产中很少用）。

> 实际上，Nginx 官方文档和开发者提到，64 个槽位足以覆盖绝大多数场景（即使每秒更新，也能保留 64 秒的历史），且内存开销小。

---

接下来是：
```c
ngx_time_update();
```
更新时间缓存
- [ngx_time_update ](https://blog.csdn.net/weixin_41812346/article/details/156053893?sharetype=blogdetail&sharerId=156053893&sharerefer=PC&sharesource=weixin_41812346&spm=1011.2480.3001.8118)

---

```c
void
ngx_time_update(void)
{
    u_char          *p0, *p1, *p2, *p3, *p4;
    ngx_tm_t         tm, gmt;
    time_t           sec;
    ngx_uint_t       msec;
    ngx_time_t      *tp;
    struct timeval   tv;

    if (!ngx_trylock(&ngx_time_lock)) {
        return;
    }

    ngx_gettimeofday(&tv);

    sec = tv.tv_sec;
    msec = tv.tv_usec / 1000;

    ngx_current_msec = ngx_monotonic_time(sec, msec);

    tp = &cached_time[slot];

    if (tp->sec == sec) {
        tp->msec = msec;
        ngx_unlock(&ngx_time_lock);
        return;
    }

    if (slot == NGX_TIME_SLOTS - 1) {
        slot = 0;
    } else {
        slot++;
    }

    tp = &cached_time[slot];

    tp->sec = sec;
    tp->msec = msec;

    ngx_gmtime(sec, &gmt);


    p0 = &cached_http_time[slot][0];

    (void) ngx_sprintf(p0, "%s, %02d %s %4d %02d:%02d:%02d GMT",
                       week[gmt.ngx_tm_wday], gmt.ngx_tm_mday,
                       months[gmt.ngx_tm_mon - 1], gmt.ngx_tm_year,
                       gmt.ngx_tm_hour, gmt.ngx_tm_min, gmt.ngx_tm_sec);

#if (NGX_HAVE_GETTIMEZONE)

    tp->gmtoff = ngx_gettimezone();
    ngx_gmtime(sec + tp->gmtoff * 60, &tm);

#elif (NGX_HAVE_GMTOFF)

    ngx_localtime(sec, &tm);
    cached_gmtoff = (ngx_int_t) (tm.ngx_tm_gmtoff / 60);
    tp->gmtoff = cached_gmtoff;

#else

    ngx_localtime(sec, &tm);
    cached_gmtoff = ngx_timezone(tm.ngx_tm_isdst);
    tp->gmtoff = cached_gmtoff;

#endif


    p1 = &cached_err_log_time[slot][0];

    (void) ngx_sprintf(p1, "%4d/%02d/%02d %02d:%02d:%02d",
                       tm.ngx_tm_year, tm.ngx_tm_mon,
                       tm.ngx_tm_mday, tm.ngx_tm_hour,
                       tm.ngx_tm_min, tm.ngx_tm_sec);


    p2 = &cached_http_log_time[slot][0];

    (void) ngx_sprintf(p2, "%02d/%s/%d:%02d:%02d:%02d %c%02i%02i",
                       tm.ngx_tm_mday, months[tm.ngx_tm_mon - 1],
                       tm.ngx_tm_year, tm.ngx_tm_hour,
                       tm.ngx_tm_min, tm.ngx_tm_sec,
                       tp->gmtoff < 0 ? '-' : '+',
                       ngx_abs(tp->gmtoff / 60), ngx_abs(tp->gmtoff % 60));

    p3 = &cached_http_log_iso8601[slot][0];

    (void) ngx_sprintf(p3, "%4d-%02d-%02dT%02d:%02d:%02d%c%02i:%02i",
                       tm.ngx_tm_year, tm.ngx_tm_mon,
                       tm.ngx_tm_mday, tm.ngx_tm_hour,
                       tm.ngx_tm_min, tm.ngx_tm_sec,
                       tp->gmtoff < 0 ? '-' : '+',
                       ngx_abs(tp->gmtoff / 60), ngx_abs(tp->gmtoff % 60));

    p4 = &cached_syslog_time[slot][0];

    (void) ngx_sprintf(p4, "%s %2d %02d:%02d:%02d",
                       months[tm.ngx_tm_mon - 1], tm.ngx_tm_mday,
                       tm.ngx_tm_hour, tm.ngx_tm_min, tm.ngx_tm_sec);

    ngx_memory_barrier();

    ngx_cached_time = tp;
    ngx_cached_http_time.data = p0;
    ngx_cached_err_log_time.data = p1;
    ngx_cached_http_log_time.data = p2;
    ngx_cached_http_log_iso8601.data = p3;
    ngx_cached_syslog_time.data = p4;

    ngx_unlock(&ngx_time_lock);
}
```

同样我的是

```c
night_time_update();
```
`night_time_update` 函数就定义在
`night_time_init` 后面

```c
    if (!ngx_trylock(&ngx_time_lock)) {
        return;
    }
```
- [ngx_trylock](https://blog.csdn.net/weixin_41812346/article/details/156055715?spm=1011.2415.3001.5331)

`ngx_trylock` 定义在
`·/nginx-1.24.0/src/os/unix/ngx_atomic.h`

`os` 目录下的内容代表着与 操作系统(operation system) 相关

`os` 目录下有 `unix` 和 `win32` 2个子目录，分别代表着 类unix操作系统(如 linux),和 windows 操作系统

nginx 的源码中常用 条件编译来进行跨平台，根据当前的环境的不同，成立的条件编译不同，最终执行的代码也不同，以此来兼容各个不同的平台。

此次我只关注 linux 环境下的实现方式

类似的 创建 `./night/src/os/unix/night_atomic.h` 文件来定义
```c
#define night_trylock(lock)  	(*(lock) == 0 && night_atomic_cmp_set(lock, 0, 1))
#define night_unlock(lock)    	*(lock) = 0
```

`atomic` 单词的含义是 原子的

`night_atomic.h` 文件中的内容就是关于原子操作的

```
原子操作（Atomic Operation）是指在执行过程中不可被中断的操作——要么完全执行成功，要么完全不执行，不会出现“执行到一半被其他线程或进程打断”的中间状态

在多线程(或多进程)程序中，多个线程可能同时访问和修改同一个共享变量。
如果操作不是原子的，就可能出现竞态条件（Race Condition），导致结果错误
```

`./night/src/os/unix/night_os_config.h`

在这个头文件中集合 `unix` 这个目录下所有的头文件，所有需要声明的内容，

统一放到 `night_os_config.h`， `night_core.h`中只需要写一个
```c
#include "night_os_config.h"
```
就引入了所有的声明，然后每个 `.c`文件 都只需要在开头写一行
```c
#include "night_core.h"
```
就包含了所有需要的声明


---

接下来是：
```c
    ngx_gettimeofday(&tv);

    sec = tv.tv_sec;
    msec = tv.tv_usec / 1000;
```
- [ngx_gettimeofday](https://blog.csdn.net/weixin_41812346/article/details/156056301?sharetype=blogdetail&sharerId=156056301&sharerefer=PC&sharesource=weixin_41812346&spm=1011.2480.3001.8118)

同样的创建 
`./night/src/os/unix/night_time.h`
定义
```c
#define night_gettimeofday(tp)  (void) gettimeofday(tp, NULL);
```
`night_time.h` `night_time.c`

中的内容是关于时间的，

与 `core/night_times.c`相比，`night_time.h` `night_time.c`中是要调用系统函数的，

`core/night_times.c` 中的内容通过调用 `night_time.h` `night_time.c` 中的内容来调用系统函数，

把调用系统函数的部分分离出来，这在 `nginx` 中是为了兼容不同的平台

`os/unix` `os/win32` 的目录下存在相同文件名的文件，其中的函数也一样，

但具体的实现是针对不同操作系统的

---

接下来是：
```c
ngx_current_msec = ngx_monotonic_time(sec, msec);
```

```c
static ngx_msec_t
ngx_monotonic_time(time_t sec, ngx_uint_t msec)
{
#if (NGX_HAVE_CLOCK_MONOTONIC)
    struct timespec  ts;

#if defined(CLOCK_MONOTONIC_FAST)
    clock_gettime(CLOCK_MONOTONIC_FAST, &ts);
#else
    clock_gettime(CLOCK_MONOTONIC, &ts);
#endif

    sec = ts.tv_sec;
    msec = ts.tv_nsec / 1000000;

#endif

    return (ngx_msec_t) sec * 1000 + msec;
}
```

- [ngx_monotonic_time](https://blog.csdn.net/weixin_41812346/article/details/156056574?sharetype=blogdetail&sharerId=156056574&sharerefer=PC&sharesource=weixin_41812346&spm=1011.2480.3001.8118)


同样地在 night_times.c 文件的顶部定义
```c
volatile 	uint64_t		night_current_msec;
```
全局变量

```c
night_current_msec
```
用来记录单调递增的 毫秒数

事件是否超时就用它来计算

```
static uint64_t
night_monotonic_time()
```
函数就定义在 `night_times.c` 中

它只用在本源文件中被调用，在其他源文件中没有调用

所以使用 `static` 修饰

相比 `nginx` 的函数定义
```c
static ngx_msec_t
ngx_monotonic_time(time_t sec, ngx_uint_t msec)
```

`ngx_msec_t` 的定义
```c
./nginx-1.24.0/ngx_files.h
typedef ngx_rbtree_key_t ngx_msec_t;
```
```c
./nginx-1.24.0/ngx_files.h
typedef ngx_uint_t ngx_rbtree_key_t;
```
```c
./nginx-1.24.0/ngx_files.h
typedef uintptr_t ngx_uint_t;
```
`uintptr_t` 在 64 位的环境下也就是 64 位无符号整数

我这里就不再重定义一个别名了，直接使用 `uint64_t`

而对于函数参数
```c
static ngx_msec_t
ngx_monotonic_time(time_t sec, ngx_uint_t msec)
```
分析函数的具体实现就知道这个函数并不需要这2个参数来传递数据

在 `ngx_monotonic_time` 函数中，这 2 个参数完全是作为局部变量来使用的

于是我的函数定义就直接是
```c
static uint64_t
night_monotonic_time()
```

---

接下来是：
```c
    tp = &cached_time[slot];

    if (tp->sec == sec) {
        tp->msec = msec;
        ngx_unlock(&ngx_time_lock);
        return;
    }

    if (slot == NGX_TIME_SLOTS - 1) {
        slot = 0;
    } else {
        slot++;
    }

    tp = &cached_time[slot];

    tp->sec = sec;
    tp->msec = msec;
```
在同一秒内只更新 毫秒数

否则，更新时间记录到下一个槽位

---

接下来是：

```c
ngx_gmtime(sec, &gmt);
```
- [ngx_gmtime](https://blog.csdn.net/weixin_41812346/article/details/156058797?sharetype=blogdetail&sharerId=156058797&sharerefer=PC&sharesource=weixin_41812346&spm=1011.2480.3001.8118)

同样在 `night_times.c` 中定义此函数

```c
void
night_gmtime(time_t t, struct tm *tp)
```
相比
```c
void
ngx_gmtime(time_t t, ngx_tm_t *tp)
```
由于
```c
./nginx-1.24.0/ngx_files.h
typedef struct tm ngx_tm_t;
```
所以我直接使用 
`struct tm`

---

接下来是：
```c
    p0 = &cached_http_time[slot][0];

    (void) ngx_sprintf(p0, "%s, %02d %s %4d %02d:%02d:%02d GMT",
                       week[gmt.ngx_tm_wday], gmt.ngx_tm_mday,
                       months[gmt.ngx_tm_mon - 1], gmt.ngx_tm_year,
                       gmt.ngx_tm_hour, gmt.ngx_tm_min, gmt.ngx_tm_sec);
```
格式化为 HTTP 格式时间，`cached_http_time` 数组和 `cached_time` 数组类似，

存储的是 `cached_time`对应元素 时间记录 格式化为 HTTP 格式时间的字符串的结果 

---

接下来是：
```c
    ngx_localtime(sec, &tm);
    cached_gmtoff = (ngx_int_t) (tm.ngx_tm_gmtoff / 60);
    tp->gmtoff = cached_gmtoff;
```
时区处理部分

- [ngx_localtime](https://blog.csdn.net/weixin_41812346/article/details/156063156?sharetype=blogdetail&sharerId=156063156&sharerefer=PC&sharesource=weixin_41812346&spm=1011.2480.3001.8118)

同样的在 `./night/src/os/unix/night_time.c` 中定义

```c
void
night_localtime(time_t s, struct tm *tm)
```

---

接下来是：
```c
    p1 = &cached_err_log_time[slot][0];

    (void) ngx_sprintf(p1, "%4d/%02d/%02d %02d:%02d:%02d",
                       tm.ngx_tm_year, tm.ngx_tm_mon,
                       tm.ngx_tm_mday, tm.ngx_tm_hour,
                       tm.ngx_tm_min, tm.ngx_tm_sec);


    p2 = &cached_http_log_time[slot][0];

    (void) ngx_sprintf(p2, "%02d/%s/%d:%02d:%02d:%02d %c%02i%02i",
                       tm.ngx_tm_mday, months[tm.ngx_tm_mon - 1],
                       tm.ngx_tm_year, tm.ngx_tm_hour,
                       tm.ngx_tm_min, tm.ngx_tm_sec,
                       tp->gmtoff < 0 ? '-' : '+',
                       ngx_abs(tp->gmtoff / 60), ngx_abs(tp->gmtoff % 60));

    p3 = &cached_http_log_iso8601[slot][0];

    (void) ngx_sprintf(p3, "%4d-%02d-%02dT%02d:%02d:%02d%c%02i:%02i",
                       tm.ngx_tm_year, tm.ngx_tm_mon,
                       tm.ngx_tm_mday, tm.ngx_tm_hour,
                       tm.ngx_tm_min, tm.ngx_tm_sec,
                       tp->gmtoff < 0 ? '-' : '+',
                       ngx_abs(tp->gmtoff / 60), ngx_abs(tp->gmtoff % 60));

    p4 = &cached_syslog_time[slot][0];

    (void) ngx_sprintf(p4, "%s %2d %02d:%02d:%02d",
                       months[tm.ngx_tm_mon - 1], tm.ngx_tm_mday,
                       tm.ngx_tm_hour, tm.ngx_tm_min, tm.ngx_tm_sec);

```
其他的时间格式字符串的格式化

在我的 `night_time_update` 中

其他的时间格式字符串的格式化先搁置，等遇到其用途，能明确其用途再考虑

---

接下来是：
```c
ngx_memory_barrier();
```
- [ngx_memory_barrier](https://blog.csdn.net/weixin_41812346/article/details/156063383?sharetype=blogdetail&sharerId=156063383&sharerefer=PC&sharesource=weixin_41812346&spm=1011.2480.3001.8118)

同样的在 `./night/src/os/unix/night_atomic.h`

中定义
```c
#define night_memory_barrier()	__sync_synchronize()
```

---

接下来是：
```c
    ngx_cached_time = tp;

    ngx_cached_http_time.data = p0;
    ngx_cached_err_log_time.data = p1;
    ngx_cached_http_log_time.data = p2;
    ngx_cached_http_log_iso8601.data = p3;
    ngx_cached_syslog_time.data = p4;

    ngx_unlock(&ngx_time_lock);
```

`ngx_time_update` 函数到此结束

`ngx_time_init` 函数也到此结束

---