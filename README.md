# Agumi
[![test](https://github.com/zanllp/agumi/actions/workflows/test.yml/badge.svg)](https://github.com/zanllp/agumi/actions/workflows/test.yml)

可作为c++的内嵌语言或单独的语言使用。包含闭包，垃圾回收，函数是一等公民，事件循环，自定义运算符重载等特性，针对网络请求及服务器也提供了异步接口。
## 编译
可选的ssl支持
### 启用ssl支持
agumi的网络io通过调用[sion](https://github.com/zanllp/sion)来实现，sion的https支持需要openssl，这边先使用[vcpkg](https://github.com/microsoft/vcpkg)安装openssl，再
```shell
cmake -S src -B bin -DVCPKG_PATH=/home/vcpkg # /home/vcpkg替换变成自己的vcpkg文件夹
cmake --build bin
```
### 不使用openssl
仍可进行http协议的请求，不需要c++17标准库外的其他东西
```shell
cmake -S src -B bin
cmake --build bin
```
## 运行
```shell
./bin/agumi
```
## 例子
1. 进入repl
```shell
./bin/agumi -repl
```
2. 斐波那契
```js
const fib = (a) => (a>1) ? (fib(a-1) + fib(a-2)) : a
fib(10)
```
![image](https://user-images.githubusercontent.com/25872019/118397323-ff982a00-b685-11eb-9bf6-897e5ea5c23e.png)

## 启动参数
### -exec=[file path]
执行文件

![image](https://user-images.githubusercontent.com/25872019/119235882-2b744d80-bb67-11eb-9b2a-57444f2dc294.png)
### -repl
进入repl
### -test
运行测试
## 几个测试和例子
### 闭包
闭包测试和使用[点这里](./script/test/closure.as)
### promise
使用纯agumi实现的简陋的promise[点这里](./script/lib/promise.as)
### 键值存储
[点这里](./script/lib/kv.as)
### tcp & http 服务器
两个服务器都是异步非阻塞的，tcp是调用的本地c++的实现，http是基于tcp使用纯agumi写的，当然依旧存在一些问题
#### tcp
 [使用参考](./script/tcp_server.as)
#### http
 [具体实现](./script/lib/server/http_server.as)
 
 一个例子 简单的文件读取
 [源码点这](./script/http_server.as)
```bash
# 启动服务器

agumi -exec=script/http_server.as
# 浏览器打开 http://127.0.0.1:9999/?path=lib/server/http_status_map.csv ，查看效果
```
## 内置函数 / 如何与c++ bind / 基础类型运算符重载
参考[pre define func](./src/PreDefineFn.h)
### 例子：使用运算符重载实现函数式语言pipe的功能
```js
['array','number','boolean','object','string', 'null'].select(type => {
    define_operator(type, 'function', '->', (l, r) => r(l))
})
define_operator('function', 'function', '->', (l,r) => (a) =>  r(l(a)))
```

<img  alt="image" src="https://user-images.githubusercontent.com/25872019/155872046-edbf1705-3630-48f5-91ae-d91d2a467c34.png">

# 垃圾回收
默认不启用gc,如果需要长时间启用agumi，需要gc[参考这个文件来启用](./script/test/gc_stress.as)
### 效果参考
<img width="523" alt="image" src="https://user-images.githubusercontent.com/25872019/159162421-1cb64080-a085-4f10-b70e-e5459c60dc56.png">

## 已知问题
1. 运算符优先级有时会表现异常，后面改用调度场重新写
