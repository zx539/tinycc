#!/bin/sh
# Basic LA32R object-file regression checks.
set -eu

tcc=${1:-./la32r-tcc}
tmp=${TMPDIR:-/tmp}/tcc-la32r-test.$$
trap 'rm -f "$tmp.c" "$tmp.o" "$tmp-call.c" "$tmp-call.o" "$tmp-ll.c" "$tmp-ll.o" "$tmp-link.c" "$tmp-exe" "$tmp.h" "$tmp.r" "$tmp.call-r" "$tmp.ll-s" "$tmp.exe-h" "$tmp.exe-r" "$tmp.exe-u"' EXIT HUP INT TERM

cat >"$tmp.c" <<'EOF'
int add(int a, int b) { return a + b; }
int branch(int a, int b) {
    if (a < b) return 1;
    if ((unsigned)a >= (unsigned)b) return 2;
    return 3;
}
int constant(void) { return 0x12345678; }
int ninth(int a, int b, int c, int d, int e, int f, int g, int h, int i) {
    return i;
}
int call_ninth(void) { return ninth(1, 2, 3, 4, 5, 6, 7, 8, 9); }
int call_indirect(int (*fn)(int), int x) { return fn(x); }
int recurse(int n) { return n ? recurse(n - 1) + 1 : 0; }
EOF
"$tcc" -c "$tmp.c" -o "$tmp.o"
readelf -h "$tmp.o" >"$tmp.h"
grep -q 'Class:.*ELF32' "$tmp.h"
grep -q 'Machine:.*LoongArch' "$tmp.h"
cat >"$tmp-call.c" <<'EOF'
extern int ext(int);
int call_ext(int x) { return ext(x); }
EOF
"$tcc" -c "$tmp-call.c" -o "$tmp-call.o"
readelf -r "$tmp-call.o" >"$tmp.call-r"
grep -q 'R_LARCH_B26' "$tmp.call-r"

cat >"$tmp-ll.c" <<'EOF'
unsigned long long quotient(unsigned long long a, unsigned long long b) {
    return a / b;
}
unsigned long long sum(unsigned long long a, unsigned long long b) {
    return a + b;
}
unsigned long long product(unsigned long long a, unsigned long long b) {
    return a * b;
}
EOF
"$tcc" -c "$tmp-ll.c" -o "$tmp-ll.o"
nm "$tmp-ll.o" >"$tmp.ll-s"
grep -q '__udivdi3' "$tmp.ll-s"
grep -q '__muldi3' "$tmp.ll-s"

archive=$(dirname "$tcc")/la32r-libtcc1.a
test -s "$archive"
nm "$archive" | grep -q '__udivdi3'
nm "$archive" | grep -q '__muldi3'

cat >"$tmp-link.c" <<'EOF'
volatile unsigned int global = 0x12345678;
unsigned int *global_ptr = (unsigned int *)&global;
unsigned long long multiply(unsigned long long a, unsigned long long b) {
    return a * b;
}
int _start(void) { return global + *global_ptr + multiply(6, 7); }
EOF
"$tcc" -nostdlib -static "$tmp-link.c" "$archive" -o "$tmp-exe"
readelf -h "$tmp-exe" >"$tmp.exe-h"
grep -q 'Type:.*EXEC' "$tmp.exe-h"
grep -q 'Machine:.*LoongArch' "$tmp.exe-h"
readelf -r "$tmp-exe" >"$tmp.exe-r"
grep -q 'There are no relocations' "$tmp.exe-r"
readelf -s "$tmp-exe" >"$tmp.exe-u"
if grep -q 'GLOBAL.*UND' "$tmp.exe-u"; then
    echo 'LA32R executable has unresolved global symbols' >&2
    exit 1
fi

echo 'LA32R object and static-link checks passed'
