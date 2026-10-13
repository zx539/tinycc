# LA32R 移植后续工作

状态：进行中

当前分支已经可以构建 `la32r-tcc`，并生成 ELF32 LoongArch 可重定位目标文件。现有
后端覆盖 LA32R 基础整数路径，但还不能作为完整的 LoongArch32 C 工具链使用。

## 待完成内容

- 按 LA32R ABI 完善参数传递、返回值、栈帧和被调用者保存寄存器处理。
- 实现浮点指令、浮点比较、浮点转换和浮点寄存器分配。
- 补齐 `libtcc1.a` 所需的整数、浮点和编译器辅助函数运行库。
- 实现 LA32R 内联汇编寄存器、操作数和指令解析。
- 补齐绝对地址、PC 相对地址、GOT/PLT、TLS 等 ELF 重定位及动态链接路径。
- 增加 LA32R 汇编输出、反汇编和目标文件回归测试。
- 在真实 LoongArch32 仿真器或硬件上运行编译、链接和运行时测试。

## 验收标准

```sh
./configure --cpu=la32r --config-bcheck=no --config-backtrace=no
make la32r-tcc
make -C lib CROSS_TARGET=la32r
```

上述命令应成功完成，并且简单整数 C 程序应生成 `ELF32 LoongArch` 目标文件。完成
完整 ABI 和运行库后，再增加可执行文件链接及目标平台运行测试。
