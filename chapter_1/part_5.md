 继续 main 函数分析
 ```c
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
```

---

### 1. 正则表达式（PCRE）初始化

```c
#if (NGX_PCRE)
    ngx_regex_init();
#endif
```
- 这是一个**条件编译块**，仅在 Nginx 编译时启用了 PCRE（Perl Compatible Regular Expressions）支持时才会执行。
- `NGX_PCRE` 是一个宏，当用户配置 Nginx 时使用了 `--with-pcre` 或系统默认支持正则时启用。

#### `ngx_regex_init()` 做了什么？
- 初始化 PCRE 库的全局状态。
- 在支持 JIT（Just-In-Time compilation）的平台上，会尝试启用 JIT 编译以加速正则匹配。
- 设置 Nginx 内部的正则缓存机制
- 若未初始化，后续使用 `location ~ /xxx/` 或 `rewrite` 等带正则的指令将无法工作。

#### 设计意义：
- **按需初始化**：避免在不需要正则功能的轻量部署中引入额外开销。
- **早期初始化**：虽然此时还未加载配置，但某些模块（如核心 HTTP 模块）在 `ngx_init_cycle` 中会立即使用正则，因此必须在配置解析前完成初始化。

---

这次先不使用 正则 功能，暂时跳过这部分，

继续看后面的代码

---

### 2. 获取当前进程 ID 和父进程 ID

```c
ngx_pid = ngx_getpid();
ngx_parent = ngx_getppid();
```

#### 逻辑与意图：
- 调用封装后的系统函数获取当前进程的 PID（Process ID）和父进程的 PPID（Parent Process ID）。
- `ngx_getpid()` 封装 `getpid()`
- `ngx_getppid()` 封装 `getppid()`。

在 `./nginx-1.24.0/src/os/unix/ngx_process.h`中
```c
#define ngx_getpid   getpid
#define ngx_getppid  getppid
```
`process` 有进程的含义

`ngx_getpid` 的作用是获取进程 `id`,

所以它的定义放在 `ngx_process.h`中

它是对系统函数的封装，当前的操作系统是 linux，

所以， `ngx_process.h` 被放在 `os/unix` 目录下

---

####  存储位置：
- `ngx_pid` 和 `ngx_parent` 是 **全局变量**

定义在 `./nginx-1.24.0/src/os/unix/ngx_process_cycle.c`
```c
ngx_pid_t     ngx_pid;
ngx_pid_t     ngx_parent;
```
`ngx_pid_t` 定义在
`./nginx-1.24.0/src/os/unix/ngx_process.h`
```c
typedef pid_t       ngx_pid_t;
```
是对 `pid_t` 的重定义

在 C 语言中，`pid_t` 类型 是一个专门用于表示进程 ID（Process ID） 的数据类型

---

同样的 在我的 `night.c` 中的 `main` 函数中
```c
    night_pid = getpid();
    night_parent = getppid();
```
直接调用系统函数，

全局变量 `night_pid` `night_parent`就定义在
`night.c`的顶部
```c
pid_t 			night_pid;
pid_t 			night_parent;
```

---

### 3. 初始化错误日志系统

```c
log = ngx_log_init(ngx_prefix, ngx_error_log);
if (log == NULL) {
    return 1;
}
```
#### 逻辑与意图：
- 调用 `ngx_log_init()` 初始化 Nginx 的**核心日志系统**。
- 参数说明：
  - `ngx_prefix`：Nginx 安装前缀路径（如 `/usr/local/nginx`），用于解析相对路径。
  - `ngx_error_log`：用户通过 `-e` 指定或配置文件中设置的错误日志路径（默认为 `logs/error.log`）。

####  `ngx_log_init()` 内部做了什么？
1. 将 `ngx_error_log` 路径与 `ngx_prefix` 合并，生成绝对路径。
2. 创建或打开该日志文件（通常以追加模式 `O_APPEND | O_CREAT` 打开）。
3. 分配一个 `ngx_log_t` 结构体，设置其 `file->fd` 为日志文件描述符。
4. 设置日志级别（默认 `NGX_LOG_ERR`，可通过 `-g "error_log ...;"` 覆盖）。
5. 返回指向该 `ngx_log_t` 的指针。

#### ❗ 为什么失败要退出？
- 日志是 Nginx **诊断问题的唯一出口**。如果连日志都无法初始化（如权限不足、路径不存在），后续任何错误都无法记录，程序应立即终止。

---

日志部分不是核心，先暂时跳过

---

### 4. OpenSSL 初始化（条件编译）

```c
/* STUB */
#if (NGX_OPENSSL)
    ngx_ssl_init(log);
#endif
```

#### 逻辑与意图：
- 又一个**条件编译块**，仅在编译时启用了 SSL/TLS 支持（如使用 `--with-http_ssl_module`）时生效。
- `NGX_OPENSSL` 表示底层使用 OpenSSL 库（Nginx 也支持 BoringSSL、LibreSSL，但宏名可能不同）。

#### `ngx_ssl_init(log)` 做了什么？
- 初始化 OpenSSL 库的全局状态：
  - 调用 `OPENSSL_init_ssl()` 或旧版 `SSL_library_init()`。
  - 初始化加密算法、错误字符串表。
  - **设置线程回调函数**（若启用了多线程）：OpenSSL 早期版本非线程安全，需 Nginx 提供锁机制（通过 `CRYPTO_set_locking_callback` 等）。
  - 可能初始化随机数生成器（PRNG）。
- 所有操作的错误信息会通过传入的 `log` 输出。

---

暂且不使用 SSL，先跳过