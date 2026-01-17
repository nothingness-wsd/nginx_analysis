继续 main 函数分析

```c
/* TODO */ ngx_max_sockets = -1;
```
初始化最大 socket 数量，-1 表示未设置
后续会根据系统限制设置

---

```c
ngx_time_init();
```
时间系统初始化

- [ngx_time_init](https://blog.csdn.net/weixin_41812346/article/details/156053372?sharetype=blogdetail&sharerId=156053372&sharerefer=PC&sharesource=weixin_41812346&spm=1011.2480.3001.8118)

---

同样的创建 `night/src/core/night_times.c`,
`night/src/core/night_times.h`

在 `night_times.c` 中定义 `night_time_init` 函数

对于预设各种时间格式的字符串的长度这一部分，只保留
```c
night_cached_http_time.len = sizeof("Mon, 28 Sep 1970 06:00:00 GMT") - 1;
```
`cached_http_time` 用于在 HTTP 响应头中快速输出标准时间，避免每次格式化

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
而是 night_time_t ，那就需要 night_time_t 的完整定义 也就是 night_time_s{} 部分的定义写在 a.h 之前，
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

- [ngx_time_update ](https://blog.csdn.net/weixin_41812346/article/details/156053893?sharetype=blogdetail&sharerId=156053893&sharerefer=PC&sharesource=weixin_41812346&spm=1011.2480.3001.8118)

同样我的是

```c
night_time_update();
```
`night_time_update` 函数就定义在后面

---

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
就引入了所有的声明，然后每个 `.c` 都只需要在开头写一行
```c
#include "night_core.h"
```
就包含了所有需要的声明


---

```c
ngx_gettimeofday(&tv);
```
- [ngx_gettimeofday](https://blog.csdn.net/weixin_41812346/article/details/156056301?sharetype=blogdetail&sharerId=156056301&sharerefer=PC&sharesource=weixin_41812346&spm=1011.2480.3001.8118)

同样的创建 
`./night/src/os/unix/night_time.h`
定义
```c
#define night_gettimeofday(tp)  (void) gettimeofday(tp, NULL);
```

---

```c
ngx_current_msec = ngx_monotonic_time(sec, msec);
```
- [ngx_monotonic_time](https://blog.csdn.net/weixin_41812346/article/details/156056574?sharetype=blogdetail&sharerId=156056574&sharerefer=PC&sharesource=weixin_41812346&spm=1011.2480.3001.8118)



在 night_times.c 文件的顶部定义
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
部分原样 copy

---

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

其他的时间格式字符串的格式化先搁置，等遇到其用途，能明确其用途再考虑

---

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


