# Agumi
可作为c++的内嵌语言或一门单独的语言使用
[![test](https://github.com/zanllp/agumi/actions/workflows/test.yml/badge.svg)](https://github.com/zanllp/agumi/actions/workflows/test.yml)
## 编译
```shell
mkdir bin
cd bin
cmake -S ../src -B .
cmake --build . 或者 make
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
### -ast
repl时开启输出语法树
### -tokenizer
repl时开启输出令牌化
### -test
运行测试
## 内置函数 / 如何与c++ bind / 基础类型运算符重载
参考[pre define func](./src/PreDefineFn.h)
## 已知问题
1. 运算符优先级有时会表现异常，后面改用调度场重新写
