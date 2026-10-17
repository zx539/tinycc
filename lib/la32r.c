/* Integer support routines used by the LA32R target.
 *
 * Keep the implementation in 32-bit terms.  Using a native long long
 * expression here would make the compiler call these routines recursively
 * while building libtcc1.a.
 */
typedef unsigned int la_word;

typedef struct {
    la_word lo, hi;
} la_u64;

typedef union {
    unsigned long long value;
    la_u64 words;
} la_value;

static int la_ge(la_u64 left, la_u64 right)
{
    if (left.hi > right.hi)
        return 1;
    if (left.hi < right.hi)
        return 0;
    return left.lo >= right.lo;
}

/* Restoring division, scanning the dividend from bit 63 to bit 0. */
static unsigned long long la_udivmod(la_u64 dividend, la_u64 divisor,
                                     int want_remainder)
{
    la_u64 quotient;
    la_u64 rest;
    unsigned int bit;

    quotient.lo = quotient.hi = 0;
    rest.lo = rest.hi = 0;

    if (divisor.lo == 0 && divisor.hi == 0) {
        return 0;
    }

    for (bit = 64; bit != 0; --bit) {
        unsigned int n = bit - 1;
        la_word carry = rest.lo >> 31;
        rest.lo <<= 1;
        rest.hi = (rest.hi << 1) | carry;
        if (n >= 32)
            rest.lo |= (dividend.hi >> (n - 32)) & 1;
        else
            rest.lo |= (dividend.lo >> n) & 1;
        if (la_ge(rest, divisor)) {
            la_word borrow = rest.lo < divisor.lo;
            rest.lo -= divisor.lo;
            rest.hi -= divisor.hi + borrow;
            if (n >= 32)
                quotient.hi |= (la_word)1 << (n - 32);
            else
                quotient.lo |= (la_word)1 << n;
        }
    }
    if (want_remainder)
        return ((unsigned long long)rest.hi << 32) | rest.lo;
    return ((unsigned long long)quotient.hi << 32) | quotient.lo;
}

unsigned long long __udivdi3(unsigned long long dividend,
                             unsigned long long divisor)
{
    la_value left, right;
    left.value = dividend;
    right.value = divisor;
    return la_udivmod(left.words, right.words, 0);
}

unsigned long long __umoddi3(unsigned long long dividend,
                             unsigned long long divisor)
{
    la_value left, right;
    left.value = dividend;
    right.value = divisor;
    return la_udivmod(left.words, right.words, 1);
}

long long __divdi3(long long dividend, long long divisor)
{
    la_value left_value, right_value, result_value;
    la_u64 left, right, result;
    left_value.value = (unsigned long long)dividend;
    right_value.value = (unsigned long long)divisor;
    left.lo = left_value.words.lo;
    left.hi = left_value.words.hi;
    right.lo = right_value.words.lo;
    right.hi = right_value.words.hi;
    int negative = (left.hi >> 31) != (right.hi >> 31);

    if ((left.hi >> 31) != 0) {
        left.lo = ~left.lo + 1;
        left.hi = ~left.hi + (left.lo == 0);
    }
    if ((right.hi >> 31) != 0) {
        right.lo = ~right.lo + 1;
        right.hi = ~right.hi + (right.lo == 0);
    }
    result_value.value = la_udivmod(left, right, 0);
    result.lo = result_value.words.lo;
    result.hi = result_value.words.hi;
    if (negative)
        { result.lo = ~result.lo + 1; result.hi = ~result.hi + (result.lo == 0); }
    return (long long)(((unsigned long long)result.hi << 32) | result.lo);
}

long long __moddi3(long long dividend, long long divisor)
{
    la_value left_value, right_value, result_value;
    la_u64 left, right, remainder;
    left_value.value = (unsigned long long)dividend;
    right_value.value = (unsigned long long)divisor;
    left.lo = left_value.words.lo;
    left.hi = left_value.words.hi;
    right.lo = right_value.words.lo;
    right.hi = right_value.words.hi;
    int negative = (left.hi >> 31) != 0;

    if ((left.hi >> 31) != 0) {
        left.lo = ~left.lo + 1;
        left.hi = ~left.hi + (left.lo == 0);
    }
    if ((right.hi >> 31) != 0) {
        right.lo = ~right.lo + 1;
        right.hi = ~right.hi + (right.lo == 0);
    }
    result_value.value = la_udivmod(left, right, 1);
    remainder.lo = result_value.words.lo;
    remainder.hi = result_value.words.hi;
    if (negative)
        { remainder.lo = ~remainder.lo + 1; remainder.hi = ~remainder.hi + (remainder.lo == 0); }
    return (long long)(((unsigned long long)remainder.hi << 32) | remainder.lo);
}

unsigned long long __muldi3(unsigned long long left,
                            unsigned long long right)
{
    la_value a, b;
    la_word low, high, p0, p1, p2;

    a.value = left;
    b.value = right;
    p0 = (a.words.lo & 0xffff) * (b.words.lo & 0xffff);
    p1 = (a.words.lo >> 16) * (b.words.lo & 0xffff);
    p2 = (a.words.lo & 0xffff) * (b.words.lo >> 16);
    low = p0;
    high = (p1 >> 16) + (p2 >> 16);
    p1 <<= 16;
    p2 <<= 16;
    low += p1;
    high += low < p1;
    low += p2;
    high += low < p2;
    high += (a.words.lo >> 16) * (b.words.lo >> 16);
    high = (a.words.lo * b.words.hi)
         + (a.words.hi * b.words.lo)
         + high;
    return ((unsigned long long)high << 32) | low;
}

long long __ashldi3(long long value, int shift)
{
    la_value value_bits;
    la_u64 v;
    value_bits.value = (unsigned long long)value;
    v.lo = value_bits.words.lo;
    v.hi = value_bits.words.hi;
    if (shift >= 64)
        v.lo = v.hi = 0;
    else if (shift >= 32) {
        v.hi = v.lo << (shift - 32);
        v.lo = 0;
    } else if (shift != 0) {
        v.hi = (v.hi << shift) | (v.lo >> (32 - shift));
        v.lo <<= shift;
    }
    return (long long)(((unsigned long long)v.hi << 32) | v.lo);
}

unsigned long long __lshrdi3(unsigned long long value, int shift)
{
    la_value value_bits;
    la_u64 v;
    value_bits.value = value;
    v.lo = value_bits.words.lo;
    v.hi = value_bits.words.hi;
    if (shift >= 64)
        v.lo = v.hi = 0;
    else if (shift >= 32) {
        v.lo = v.hi >> (shift - 32);
        v.hi = 0;
    } else if (shift != 0) {
        v.lo = (v.lo >> shift) | (v.hi << (32 - shift));
        v.hi >>= shift;
    }
    return ((unsigned long long)v.hi << 32) | v.lo;
}

long long __ashrdi3(long long value, int shift)
{
    la_value value_bits;
    la_u64 v;
    value_bits.value = (unsigned long long)value;
    v.lo = value_bits.words.lo;
    v.hi = value_bits.words.hi;
    la_word sign = (la_word)-(v.hi >> 31);
    if (shift >= 64) {
        v.lo = v.hi = sign;
    } else if (shift >= 32) {
        v.lo = (la_word)((int)v.hi >> (shift - 32));
        v.hi = sign;
    } else if (shift != 0) {
        v.lo = (v.lo >> shift) | (v.hi << (32 - shift));
        v.hi = (la_word)((int)v.hi >> shift);
    }
    return (long long)(((unsigned long long)v.hi << 32) | v.lo);
}
