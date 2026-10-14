# LA32R ABI、运行库与目标机验证

状态：进行中

## 背景

当前 `mob` 分支已经可以构建 `la32r-tcc` 和空的 `la32r-libtcc1.a`，并通过
`tests/la32r-test.sh` 检查 ELF32 LoongArch 目标文件。基础整数指令、访存、条件分支、
简单调用和目标文件中的 `R_LARCH_B26` 已有覆盖，但生成代码还不能作为完整的
LoongArch32 C ABI 实现使用。当前提交已同步到 GitHub `zx539/tinycc` 和内网
`ZX539/tinycc-la32r` 的 `mob` 分支。

## 工作项

- [x] 按 LA32R 基本整数 ABI 使用 `a0..a7` 传递前八个整数参数，后续整数参数从入口栈帧偏移 0 开始读取。
- [x] 固定基础栈帧布局并保存/恢复 `ra` 和 `fp`，覆盖递归调用所需的返回地址状态。
- [x] 实现标量整数的直接调用、函数指针调用及超过八个参数时的栈上传递。
- [ ] 在目标机确认栈帧布局及递归、栈参数行为；补齐被调用者保存寄存器规则。
- [ ] 核对返回值、结构体参数/返回和可变参数 ABI。
- 为 `libtcc1.a` 增加至少可用的整数除法、取模、乘法和编译器辅助函数。
- 补齐绝对地址、PC 相对地址、B16/B21/B26 范围检查，以及静态链接所需的重定位。
- 在具备 LoongArch32 assembler/linker 或 QEMU 的环境中增加可执行文件和运行时测试。

本地对象测试覆盖 9 个整数参数、递归和函数指针调用。当前环境没有 LA32R
assembler、objdump 或 QEMU，参数及栈帧行为尚未经过目标机运行验证；结构体、浮点、
可变参数和超大调用帧也不在本轮实现范围内。

## 验收标准

```sh
./configure --cpu=la32r --config-bcheck=no --config-backtrace=no
make la32r-tcc
make -C lib CROSS_TARGET=la32r
tests/la32r-test.sh ./la32r-tcc
```

此外，需要在 LoongArch32 目标环境中通过以下类别的测试：整数算术和比较、局部变量、
递归、带栈参数的调用、外部函数调用、结构体返回，以及使用 `libtcc1.a` 的除法和取模。
在目标环境可用前，本地验收只覆盖编译结果、ELF 头和 relocation 内容。
