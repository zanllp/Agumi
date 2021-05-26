# Agumi
[![test](https://github.com/zanllp/agumi/actions/workflows/test.yml/badge.svg)](https://github.com/zanllp/agumi/actions/workflows/test.yml)

可作为c++的内嵌语言或一门单独的语言使用
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
### -ast
repl时开启输出语法树
### -tokenizer
repl时开启输出令牌化
### -test
运行测试
## 内置函数 / 如何与c++ bind / 基础类型运算符重载
参考[pre define func](./src/PreDefineFn.h)
### 例子：使用运算符重载实现函数式语言pipe的功能
```cpp
LocalClassDefine fn_def;
std::map<KW, std::function<JsValue(JsValue &, JsValue &)>> fn_op_def;
fn_op_def[add_] = [&](JsValue &l, JsValue &r)
{
    auto new_fn = [=, &vm](Vector<JsValue> args) -> JsValue
    {
        return vm.FuncCall(r, vm.FuncCall(l, args));
    };
    return vm.DefineFunc(new_fn);
};
fn_def.binary_operator_overload[JsType::function] = fn_op_def;
vm.class_define[JsType::function] = fn_def;
```
![image](https://user-images.githubusercontent.com/25872019/119687653-f5421100-be79-11eb-9441-95174cff6068.png)
## 已知问题
1. 运算符优先级有时会表现异常，后面改用调度场重新写
