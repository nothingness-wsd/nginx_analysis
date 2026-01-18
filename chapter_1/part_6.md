继续 main 函数的分析

```c
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
```    

---

### 注释先行：理解设计意图

```c
/*
 * init_cycle->log is required for signal handlers and
 * ngx_process_options()
 */
```

#### 作用：
- 这是一条**前置说明注释**，强调 `init_cycle.log` 必须在后续某些函数调用前就设置好。
- **为什么？**
  - **信号处理函数**可能在早期被触发（例如用户快速发送 reload 信号），而这些信号处理函数内部会使用 `ngx_cycle->log` 来记录日志。
  - `ngx_process_options()` 在处理路径时若出错（如路径非法），也需要通过 `init_cycle.log` 输出错误信息。
- 因此，必须在 `init_cycle` 初始化早期就赋予其有效的 `log` 字段。

> 这体现了 Nginx 的 **“尽早可用”原则**：核心基础设施（如日志）必须在任何可能出错或需要记录的代码执行前就绪。

---

### 1. 清零 init_cycle 结构体

```c
ngx_memzero(&init_cycle, sizeof(ngx_cycle_t));
```

#### 逻辑：
- 调用 `ngx_memzero`（本质是 `memset(ptr, 0, size)` 的封装）将栈上定义的 `init_cycle` 结构体**全部清零**。

在 `./nginx-1.24.0/src/core/ngx_string.h`中
```c
#define ngx_memzero(buf, n)       (void) memset(buf, 0, n)
#define ngx_memset(buf, c, n)     (void) memset(buf, c, n)
```

#### 意图：
- 确保 `ngx_cycle_t` 中所有字段处于**已知的初始状态（NULL/0）**。
- 避免因未初始化的垃圾值导致后续逻辑崩溃（如访问野指针）。

#### 注意：
- `init_cycle` 是**局部变量**（定义在 `main` 函数栈上），不是堆分配对象。
- 此时尚未创建内存池，因此不能动态分配 `ngx_cycle_t`，只能使用栈变量。

---

### `init_cycle` 和 `cycle` 的区别和联系？

`init_cycle` 和 `cycle` 是 Nginx 启动和运行过程中两个核心的配置上下文对象，它们都属于 `ngx_cycle_t` 类型，但在**生命周期、用途和内容**上有明确区分。

---

### 一、`init_cycle` 的作用和意图

- `init_cycle` 是在 `main()` 函数中**栈上分配的临时 `ngx_cycle_t` 对象**。
- 它用于 **Nginx 启动早期阶段**（配置文件加载之前）的初始化工作。

### 意图
> **为 `ngx_init_cycle()` 函数提供一个“种子”或“模板”，使其能基于此构建完整的运行时配置上下文（`cycle`）。**

### 主要用途包括：
1. **承载命令行参数解析结果**  
   - 如 `-p prefix`, `-c conf`, `-g directives` 等。
2. **保存原始 `argv`**  
   - 用于热升级时重新执行 Nginx 进程。
3. **标准化路径**  
   - 将相对路径（如 `conf/nginx.conf`）转换为绝对路径（如 `/usr/local/nginx/conf/nginx.conf`）。
4. **提供早期日志和内存池**  
   - 使 `ngx_process_options()`, `ngx_os_init()` 等函数能安全记录日志和分配内存。
5. **传递继承的监听套接字**（热升级场景）  
   - 通过 `NGINX` 环境变量解析出旧 master 传递的 fd。
6. **作为 `ngx_init_cycle()` 的输入参数**  
   - `cycle = ngx_init_cycle(&init_cycle);`

### 生命周期
- **仅存在于 `main()` 函数的前半段**。
- 一旦 `ngx_init_cycle()` 成功返回 `cycle`，`init_cycle` 就**不再被使用**（其内存池可能被释放或忽略）。

---

### 二、`cycle` 的作用和意图

- `cycle` 是通过 `ngx_init_cycle(&init_cycle)` **动态分配（堆上）的完整 `ngx_cycle_t` 对象**。
- 它代表 **Nginx 当前运行时的完整配置状态**。

### 意图
> **封装 Nginx 所有模块的配置、监听套接字、共享内存、连接池等运行时资源，是 master/worker 进程工作的核心上下文。**


### 生命周期
- **从 `ngx_init_cycle()` 成功后一直存在**，直到 Nginx 退出。
- 在 **reload（SIGHUP）** 时，会创建 **新的 `cycle`**，旧 `cycle` 在 worker 退出后被销毁（实现平滑重载）。
- 全局变量 `ngx_cycle` 最终指向这个 `cycle`。

---

`ngx_init_cycle()` 在创建最终的、完整的 `cycle`（即运行时上下文）时，需要一系列“前置资源”和“启动参数”，而这些资源在 `cycle` 本身被创建之前并不存在。

因此，Nginx 引入了 `init_cycle` 这个临时的、轻量级的引导上下文，专门用于收集、准备和传递这些必要信息。

`ngx_init_cycle()`在 `init_cycle`提供的资源的基础上才能够创建完整的 `cycle`

---

### 2. 设置日志指针

```c
init_cycle.log = log;
```

####  逻辑：
- 将前面通过 `ngx_log_init()` 创建的日志对象 `log` 赋值给 `init_cycle.log`。

#### 意图：
- 满足注释中提到的要求：让 `init_cycle` 拥有可用的日志输出能力。
- 后续所有对 `init_cycle` 的操作（如路径解析、内存分配失败）都可以安全地写日志。

#### 设计意义：
- **解耦日志生命周期**：日志在 `init_cycle` 之前独立创建，使其可被多个阶段复用。
- **统一错误出口**：无论是在 `ngx_process_options` 还是信号处理中，都使用同一个日志对象。

---

### 3. 将全局 ngx_cycle 指向 init_cycle（临时）

```c
ngx_cycle = &init_cycle;
```

#### 逻辑：
- `ngx_cycle` 是一个**全局变量**
- 此处将其指向当前栈上的 `init_cycle`。

全局变量不需要函数参数传递的方式就可以访问

这里的赋值是临时的，在 `cycle`创建完成后就会更新让这个全局的指针指向`cycle`

---

### 4. 为 init_cycle 创建内存池

```c
init_cycle.pool = ngx_create_pool(1024, log);
if (init_cycle.pool == NULL) {
    return 1;
}
```

#### 逻辑：
- 调用 `ngx_create_pool(1024, log)` 创建一个初始大小为 **1024 字节** 的内存池。
- 将返回的内存池指针赋给 `init_cycle.pool`。
- 若创建失败（如系统内存不足），记录错误并退出。

#### 意图：
- Nginx **几乎所有的动态内存分配都通过内存池进行**，而非直接调用 `malloc`。
- `init_cycle` 需要内存池来：
  - 存储命令行参数副本（`ngx_save_argv`）
  - 保存配置路径字符串（`ngx_process_options`）
  - 临时存储模块配置结构等

#### 为什么是 1024？
- 这是一个**保守的初始值**，足够容纳早期初始化所需的小型数据（如路径、argv 指针数组）。
- 内存池支持自动扩容（当空间不足时会分配新块），因此初始值不必过大。

#### ❗ 失败处理：
- 内存池创建失败意味着系统资源极度紧张，Nginx 无法继续启动，必须退出。

---

同样的在 `night.c`的 `main` 函数中
```c
	night_memzero(&init_cycle, sizeof(night_cycle_t));
	night_cycle = &init_cycle;
	
    init_cycle.pool = night_create_pool(1024);
    if (init_cycle.pool == NULL) 
    {	
        return 1;
    }
```    
`night_memzero`定义在 `./night/src/core/night_string.h`中
```c
#define night_memzero(buf, n)       (void) memset(buf, 0, n)
```

`night_cycle_t` 定义在
`./night/src/core/night_cycle.h`中
先定义一个框架雏形，其中具体的字段等到使用到时，明确知道字段用途时再逐个添加

---

全局变量 `night_cycle`就定义在 `night.c`文件的顶部

```c
init_cycle.pool = night_create_pool(1024);
```
创建内存池这里与 nginx 不同的是 暂时不考虑 log

---

接下来来看看
`ngx_create_pool` 的具体实现


定义在`./nginx-1.24.0/src/core/ngx_palloc.c`
```c
ngx_pool_t *
ngx_create_pool(size_t size, ngx_log_t *log)
{
    ngx_pool_t  *p;

    p = ngx_memalign(NGX_POOL_ALIGNMENT, size, log);
    if (p == NULL) {
        return NULL;
    }

    p->d.last = (u_char *) p + sizeof(ngx_pool_t);
    p->d.end = (u_char *) p + size;
    p->d.next = NULL;
    p->d.failed = 0;

    size = size - sizeof(ngx_pool_t);
    p->max = (size < NGX_MAX_ALLOC_FROM_POOL) ? size : NGX_MAX_ALLOC_FROM_POOL;

    p->current = p;
    p->chain = NULL;
    p->large = NULL;
    p->cleanup = NULL;
    p->log = log;

    return p;
}
```
- [ngx_create_pool](https://blog.csdn.net/weixin_41812346/article/details/156074108?sharetype=blogdetail&sharerId=156074108&sharerefer=PC&sharesource=weixin_41812346&spm=1011.2480.3001.8118)

---

同样的在`./night/src/core/night_palloc.c`定义 
```c
night_pool_t*
night_create_pool(size_t size)
```
与 `ngx_create_pool`相比少了 log 参数，log 的部分暂时先不考虑

在`./night/src/core/night_palloc.h`中定义`night_pool_t`

`night_palloc.c` 和 `night_palloc.h` 这 2 个文件中的内容都是与 内存池相关的内容

`palloc` 中的第一个字母 `p` 代表的是 `pool` 水池，`alloc` 代表的是 `allocate` 分配，

`palloc` 总的含义就是内存池分配

---

```c
night_pool_t  *p;
```
定义局部变量，用来保存 创建的内存池地址，也是函数要返回的返回值

```c
    p = night_memalign(NIGHT_POOL_ALIGNMENT, size);
    if (p == NULL) 
    {
        return NULL;
    }
```
分配 size 字节内存，分配失败返回 `NULL`

这和 `ngx_create_pool`是一样的

---

`night_memalign` 定义在 `/night/src/os/unix/night_alloc.c` 中

因为它需要调用系统函数，所以放在`os/unix/`目录下

- [ngx_memalign](https://blog.csdn.net/weixin_41812346/article/details/156074665?spm=1011.2415.3001.5331)

相比 `ngx_memalign` 我的 `night_memalign` 函数没有 log 部分，其他具体实现都一样

---

`NGX_POOL_ALIGNMENT` 定义在 `ngx_palloc.h`中

```c
#define NGX_POOL_ALIGNMENT       16
```

同样的在 `night_palloc.h`中定义
```c
#define NIGHT_POOL_ALIGNMENT	16
```

---

新分配的内存就把它作为一个 `night_pool_t`结构体去初始化

内存池中分别内存就是以
`night_pool_data_t`
为基本单位进行的

每一次内存池都从系统中分配来一块内存作为`night_pool_data_t`，每次调用内存池分配时就从一个数据块 `night_pool_data_t` 中分出去一部分内存

`night_pool_data_t`中的 `data` 的含义是 数据,表示这里管理，存储着有效数据

```c
struct night_pool_data_s
{
    char				*last;
    char				*end;
    night_pool_t		*next;
    uint32_t			failed;
};
```
`last` 字段记录着一个地址，这个数据块的起始地址到 `last`地址中间的内存是已经被分配出去了的内存

`end`字段也记录着一个地址，这个地址是这个数据块的边界，小于这个边界的内存地址才属于这个数据块

`next`记录的是下一个数据块的地址，这里用的是`night_pool_t`，但除了第一个数据块，后面的都是只把它作为`night_pool_data_t`，

`night_pool_t`的第一个字段是 一个`night_pool_data_t`所以拿到一个 `night_pool_t`类型但把它当作 `night_pool_data_t`使用是完全可以的

而新创建的内存池`night_pool_t`它同时也自带一个数据块，这也是理所当然的，作为一个内存池它当然至少要有一个数据块`night_pool_data_t`它才能工作啊

所以在创建时就自带一个数据块

```c
    p->d.last = (char *) p + sizeof(night_pool_t);
    p->d.end = (char *) p + size;
    p->d.next = NULL;
    p->d.failed = 0;
```
对于这第一个数据块，我们分配的内存 p,它的最前面的一小部分作为`night_pool_t`结构体来管理这个内存池，而后面的就是可以分配的数据块部分了，所以
```c
p + sizeof(night_pool_t);
```
是这个数据块的起始地址，而这个数据块当前一个字节都还未使用，所以`last`就等于这个数据块的起始地址

`end`记录这个数据块的结束边界，前面分配给`P`的内存大小是 `size`,所以
```c
p + size;
```
就是这个数据块的边界

当前只有这一个数据块，所以
```c
p->d.next = NULL;
```

`failed`字段是用来记录当前这个数据块分配失败的次数的

当一个数据块被调用来分配内存，但是这个数据块剩余的内存不够用时，这个字段就 `+1`,记录这个数据块分配失败的次数，然后找下一个数据块去分配内存

当这个数据块的失败次数较多时，那么就意味着这个数据块剩余的内存太小了，这也就意味着它下一次被调用来分配内存时失败的可能性较大，

所以当 `failed` 的值增加到一定程度后，就要采取措施，在下一次分配内存时，跳过使用这个数据块来分配的尝试，因为这样的尝试失败的可能性较大，直接从下一个数据块开始尝试分配，这样就可以减少失败的次数，节省下一点时间
```c
 p->d.failed = 0;
```
这第一个数据块还没有被调用来分配过，也就没有失败过，初始化为 0

```c
size = size - sizeof(night_pool_t);
```
`size` 是这第一个数据块的大小

```c
p->max = (size < night_pagesize) ? size : night_pagesize;
```
`max` 字段记录着一个值，大于这个值的内存分配和小于这个值的内存分配采取的是不同的方式，

小于这个值就使用前面讲到的 数据块 来分配

大于这个值就直接向系统分配，一个数据块就不太够用

这里这个值取 第一个数据块大小 和 `night_pagesize` 大小的较小值

这个值当然不能大于 数据块 本身的大小，大于数据块本身的大小就不能用数据块来分配，都不够分

这里也限制了它不超过 `night_pagesize`，超过一页大小就直接向系统分配

`night_pagesize` 代表一页内存的大小，它定义在
`./src/os/unix/night_posix_init.c`
```c
size_t  		night_pagesize = 4096;
```
初始化位 4096 字节

多数时候 64位的操作系统，一页内存的大小是 4096 字节

`night_posix_init.c`中会向操作系统获取实际的一页内存的大小，所以 `night_pagesize`定义在这里

---

```c
p->current =  p;
```
current 指向第一个数据块

前面所说的当 `failed` 字段的值增大到一定程度后要采取措施，`current`字段就是来实现这件事的

`current`字段指向一个数据块，尝试使用数据块分配内存时就从 `current`字段指向的数据块开始尝试，

位于`current`字段指向的数据块前面的数据块被认为失败可能较大，就直接跳过它们，从 `current`字段指向的数据块开始尝试分配

---

```c
    p->large = NULL;
```
`large`用来记录管理大块内存的分配(大于`p->max`的情况)

初始化为 NULL

```c
p->cleanup = NULL;
```
`p->cleanup`：
指向一个清理函数链表。

内存池销毁时会使用

初始化为 NULL

---

```c
return p;
```
返回内存池指针

`night_create_pool` 函数结束

---








