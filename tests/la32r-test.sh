#!/bin/sh
# Basic LA32R object-file regression checks.
set -eu

tcc=${1:-./la32r-tcc}
tmp=${TMPDIR:-/tmp}/tcc-la32r-test.$$
trap 'rm -f "$tmp.c" "$tmp.o" "$tmp-call.c" "$tmp-call.o" "$tmp.h" "$tmp.r" "$tmp.call-r"' EXIT HUP INT TERM

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

echo 'LA32R object checks passed'
