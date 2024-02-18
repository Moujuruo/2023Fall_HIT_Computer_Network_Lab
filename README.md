# 2023FALL_HIT_COMPUTER_NETWORK_LAB 2023秋哈工大计算机网络实验

## 总结
lab1 重构了一下指导书上的代码，使得更加工程化。
lab2 github 上原学长火炬关于 SR 协议的实现大多是有问题的，甚至完全不对。这里保证一个相对正确的 SR 协议，说相对的原因是我已经尽力修掉了大部分bug，并且连续测试数十次没有发生错误，但不排除有尚未发现的 bug。
注意 lab2 编译需要使用 MSVC，可以直接使用 Visual Studio，后面发现是文件读入读出调用以及字符数组初始化的问题会造成程序在 gcc 和 MSVC 下结果不同，问题是可以被修复的。但**这份传上来的代码**是否已经消除了这一问题我已经不记得了，大家可以自行测试一下。

## Directory Hierarchy
```
|—— lab1
|    |—— 2021113634_何栩晟_实验一.docx
|    |—— HttpHeader.cpp
|    |—— HttpHeader.h
|    |—— main.cpp
|    |—— makefile
|    |—— ProxyParam.cpp
|    |—— ProxyParam.h
|    |—— ProxyServer.cpp
|    |—— ProxyServer.h
|—— lab2
|    |—— 2021113634-何栩晟-实验二.docx
|    |—— GBN_client.cpp
|    |—— GBN_server.cpp
|    |—— markmap (3).svg
|    |—— markmap (4).svg
|    |—— SR_DOUBLE_client.cpp
|    |—— SR_DOUBLE_server.cpp
```
