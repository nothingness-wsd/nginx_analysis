
创建一个 mygrep 文件在 `Nginx`目录下
```
#!/bin/sh

rm -f ./tmp.txt

grep -rn "ngx_variable_value_t" ./nginx-1.24.0 > ./tmp.txt
```
用来查找 某个函数或宏的定义
<br>
结果输出到 `./tmp.txt` 文件中

---

在 nginx 的源码中有许多的地方使用了条件编译,而且还有许多是嵌套的条件编译，
<br>
嵌套的条件编译没有 {} 和 缩进，可读性差，不容易分辨嵌套的层次结构，
<br>
<br>
与其去分析条件是否成立，不如使用
`gcc -E` 去对源文件做预处理，来确定某条件编译的结果
<br>
如
```
gcc -E src/os/unix/ngx_files.h \
	-I src/core \
	-I src/event \
	-I src/event/modules \
	-I src/os/unix \
	-I objs \
	> ngx_files.h
```

---

