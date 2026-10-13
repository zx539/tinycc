# Tiny C Compiler

TinyCC（TCC）是一个体积小、编译速度快的 C 编译器，支持直接编译并运行 C
脚本，并包含完整的 C 预处理器。

## 构建

在 Linux、BSD 或 macOS 主机上：

```sh
./configure
make
make test
make install
```

Windows 用户请参阅 `win32/tcc-win32.txt`。生成文档需要安装 `makeinfo`。

## 已支持的目标架构

TCC 支持 i386、x86-64、ARM、AArch64 和 RISC-V64。LoongArch32（LA32R）目前处于
实验性移植阶段，可以生成 ELF32 LoongArch 可重定位目标文件：

```sh
./configure --cpu=la32r --config-bcheck=no --config-backtrace=no
make la32r-tcc
make -C lib CROSS_TARGET=la32r
```

LA32R 当前覆盖基础整数指令、基本访存、分支、函数调用和栈帧生成。浮点指令、完整
ABI 运行库、内联汇编和完整动态链接支持尚未完成，后续工作见
[`issues/0001-la32r-follow-up.md`](issues/0001-la32r-follow-up.md)。

## 示例

`examples/ex1.c` 是最简单的 Hello World 示例，其他示例展示了数学运算、斐波那契
计算和标准 C 库的使用。C 脚本可以在第一行加入：

```c
#!/usr/local/bin/tcc -run
```

## 许可证

TCC 使用 GNU Lesser General Public License，详见 `COPYING`。
