# LA32R 移植后续工作

状态：进行中

当前分支已经可以构建 `la32r-tcc`，并生成 ELF32 LoongArch 可重定位目标文件。现有
后端覆盖 LA32R 基础整数路径，但还不能作为完整的 LoongArch32 C 工具链使用。

## 已完成的基础工作

- 对齐系统 ELF ABI 中的 LA32R 重定位编号，加入 `B16`/`B21`/`B26` 基础定义。
- 修正 R-type、I12、I20、B16 和 B26 指令字段编码，支持 32 位常量的高低部分装载。
- 修正条件分支链和无条件跳转链的回填，覆盖有符号及无符号比较条件。
- 增加 `tests/la32r-test.sh`，检查 ELF32 LoongArch 头、局部代码生成和外部调用的
  `R_LARCH_B26` 重定位。

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

tests/la32r-test.sh ./la32r-tcc
```

上述命令应成功完成，回归脚本还应确认简单整数 C 程序生成 `ELF32 LoongArch` 目标
文件，并为外部函数调用生成 `R_LARCH_B26` 重定位。完成完整 ABI 和运行库后，再增加
可执行文件链接及目标平台运行测试。
