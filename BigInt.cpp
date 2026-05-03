#include "BigInt.h"
#include <stdexcept>
#include <algorithm>
#include <cassert>

using namespace std;

// ═══════════════════════════════════════════════════════════════════════════
//  Internal helpers
// ═══════════════════════════════════════════════════════════════════════════

void BigInt::trim() {
    while (limbs.size() > 1 && limbs.back() == 0)
        limbs.pop_back();
}

bool BigInt::isZero() const {
    return limbs.size() == 1 && limbs[0] == 0;
}

// Total number of decimal digits
int BigInt::numDigits() const {
    if (isZero()) return 1;
    int d = (int)(limbs.size() - 1) * BASE_DIGITS;
    int top = limbs.back();
    while (top) { d++; top /= 10; }
    return d;
}

// Operator[] returns the idx-th decimal digit (0 = least significant)
int BigInt::operator[](int idx) const {
    // find which limb and position within limb
    int limb_idx = idx / BASE_DIGITS;
    int pos      = idx % BASE_DIGITS;
    if (limb_idx >= (int)limbs.size()) return 0;
    int v = limbs[limb_idx];
    for (int i = 0; i < pos; i++) v /= 10;
    return v % 10;
}

void BigInt::fromString(const string &s) {
    limbs.clear();
    if (s.empty()) { limbs.push_back(0); return; }
    int i = (int)s.size();
    while (i > 0) {
        int start = max(0, i - BASE_DIGITS);
        string chunk = s.substr(start, i - start);
        for (char c : chunk)
            if (!isdigit(c)) throw invalid_argument("BigInt: non-digit character");
        limbs.push_back(stoi(chunk));
        i = start;
    }
    trim();
}

// Exact division by 2 (used in binary algorithms; assumes a is even OR floors)
void BigInt::divide_by_2(BigInt &a) {
    int carry = 0;
    for (int i = (int)a.limbs.size() - 1; i >= 0; i--) {
        long long cur = (long long)carry * BASE + a.limbs[i];
        a.limbs[i] = (int32_t)(cur / 2);
        carry = (int)(cur % 2);
    }
    a.trim();
}

// ═══════════════════════════════════════════════════════════════════════════
//  Constructors
// ═══════════════════════════════════════════════════════════════════════════

BigInt::BigInt() : limbs(1, 0) {}

BigInt::BigInt(long long n) {
    if (n < 0) throw invalid_argument("BigInt: negative values not supported");
    unsigned long long un = (unsigned long long)n;
    do {
        limbs.push_back((int32_t)(un % BASE));
        un /= BASE;
    } while (un);
}

BigInt::BigInt(unsigned long long n) {
    do {
        limbs.push_back((int32_t)(n % BASE));
        n /= BASE;
    } while (n);
}

BigInt::BigInt(const string &s) { fromString(s); }
BigInt::BigInt(const char *s)   { fromString(string(s)); }

// ═══════════════════════════════════════════════════════════════════════════
//  Comparison
// ═══════════════════════════════════════════════════════════════════════════

bool operator==(const BigInt &a, const BigInt &b) { return a.limbs == b.limbs; }
bool operator!=(const BigInt &a, const BigInt &b) { return !(a == b); }

bool operator<(const BigInt &a, const BigInt &b) {
    if (a.limbs.size() != b.limbs.size())
        return a.limbs.size() < b.limbs.size();
    for (int i = (int)a.limbs.size() - 1; i >= 0; i--)
        if (a.limbs[i] != b.limbs[i]) return a.limbs[i] < b.limbs[i];
    return false; // equal
}

bool operator<=(const BigInt &a, const BigInt &b) { return !(b < a); }
bool operator> (const BigInt &a, const BigInt &b) { return  (b < a); }
bool operator>=(const BigInt &a, const BigInt &b) { return !(a < b); }

// ═══════════════════════════════════════════════════════════════════════════
//  Increment / Decrement
// ═══════════════════════════════════════════════════════════════════════════

BigInt &BigInt::operator++() {
    int i = 0;
    while (i < (int)limbs.size() && limbs[i] == BASE - 1) {
        limbs[i] = 0; i++;
    }
    if (i == (int)limbs.size()) limbs.push_back(1);
    else limbs[i]++;
    return *this;
}
BigInt BigInt::operator++(int) { BigInt t(*this); ++(*this); return t; }

BigInt &BigInt::operator--() {
    if (isZero()) throw underflow_error("BigInt: decrement below zero");
    int i = 0;
    while (limbs[i] == 0) { limbs[i] = BASE - 1; i++; }
    limbs[i]--;
    trim();
    return *this;
}
BigInt BigInt::operator--(int) { BigInt t(*this); --(*this); return t; }

// ═══════════════════════════════════════════════════════════════════════════
//  Addition
// ═══════════════════════════════════════════════════════════════════════════

BigInt &operator+=(BigInt &a, const BigInt &b) {
    if (b.limbs.size() > a.limbs.size())
        a.limbs.resize(b.limbs.size(), 0);
    int64_t carry = 0;
    for (int i = 0; i < (int)b.limbs.size() || carry; i++) {
        if (i == (int)a.limbs.size()) a.limbs.push_back(0);
        int64_t s = (int64_t)a.limbs[i] + carry;
        if (i < (int)b.limbs.size()) s += b.limbs[i];
        a.limbs[i] = (int32_t)(s % BigInt::BASE);
        carry = s / BigInt::BASE;
    }
    return a;
}
BigInt operator+(const BigInt &a, const BigInt &b) {
    BigInt t(a); t += b; return t;
}

// ═══════════════════════════════════════════════════════════════════════════
//  Subtraction  (a >= b assumed; throws otherwise)
// ═══════════════════════════════════════════════════════════════════════════

BigInt &operator-=(BigInt &a, const BigInt &b) {
    if (a < b) throw underflow_error("BigInt: subtraction underflow");
    int64_t borrow = 0;
    for (int i = 0; i < (int)b.limbs.size() || borrow; i++) {
        int64_t s = (int64_t)a.limbs[i] - borrow;
        if (i < (int)b.limbs.size()) s -= b.limbs[i];
        if (s < 0) { s += BigInt::BASE; borrow = 1; }
        else        borrow = 0;
        a.limbs[i] = (int32_t)s;
    }
    a.trim();
    return a;
}
BigInt operator-(const BigInt &a, const BigInt &b) {
    BigInt t(a); t -= b; return t;
}

// ═══════════════════════════════════════════════════════════════════════════
//  Multiplication — schoolbook O(n²)
// ═══════════════════════════════════════════════════════════════════════════

BigInt BigInt::schoolbook_mul(const BigInt &a, const BigInt &b) {
    int n = a.numLimbs(), m = b.numLimbs();
    BigInt res;
    res.limbs.assign(n + m, 0);
    for (int i = 0; i < n; i++) {
        int64_t carry = 0;
        for (int j = 0; j < m || carry; j++) {
            int64_t cur = (int64_t)res.limbs[i + j] + carry;
            if (j < m) cur += (int64_t)a.limbs[i] * b.limbs[j];
            res.limbs[i + j] = (int32_t)(cur % BASE);
            carry = cur / BASE;
        }
    }
    res.trim();
    return res;
}

// ═══════════════════════════════════════════════════════════════════════════
//  Multiplication — Karatsuba O(n^log2(3)) ≈ O(n^1.585)
//
//  Split each operand at limb index 'half':
//    A = A1 * B^half + A0
//    B = B1 * B^half + B0
//  Then:
//    A*B = A1*B1 * B^(2*half)
//        + (A1*B0 + A0*B1) * B^half
//        + A0*B0
//  Middle term computed as:
//    (A0+A1)*(B0+B1) - A1*B1 - A0*B0
// ═══════════════════════════════════════════════════════════════════════════

// Helper: left-shift by 'shift' limbs (multiply by BASE^shift)
static BigInt shiftLimbs(const BigInt &a, int shift) {
    if (a.isZero()) return a;
    BigInt res;
    res.limbs.resize(a.limbs.size() + shift, 0);
    for (int i = 0; i < (int)a.limbs.size(); i++)
        res.limbs[i + shift] = a.limbs[i];
    return res;
}

// Helper: split BigInt into (high, low) at limb index 'at'
static pair<BigInt,BigInt> splitAt(const BigInt &a, int at) {
    BigInt lo, hi;
    lo.limbs.clear(); hi.limbs.clear();
    for (int i = 0; i < at && i < (int)a.limbs.size(); i++)
        lo.limbs.push_back(a.limbs[i]);
    for (int i = at; i < (int)a.limbs.size(); i++)
        hi.limbs.push_back(a.limbs[i]);
    if (lo.limbs.empty()) lo.limbs.push_back(0);
    if (hi.limbs.empty()) hi.limbs.push_back(0);
    lo.trim(); hi.trim();
    return {hi, lo};
}

BigInt BigInt::karatsuba(const BigInt &a, const BigInt &b) {
    int n = a.numLimbs(), m = b.numLimbs();
    // Base case: fall back to schoolbook for small inputs
    if (n < KARA_THRESHOLD || m < KARA_THRESHOLD)
        return schoolbook_mul(a, b);

    int half = max(n, m) / 2;

    auto [a1, a0] = splitAt(a, half);
    auto [b1, b0] = splitAt(b, half);

    BigInt z2 = karatsuba(a1, b1);                    // a1 * b1
    BigInt z0 = karatsuba(a0, b0);                    // a0 * b0
    BigInt z1 = karatsuba(a1 + a0, b1 + b0) - z2 - z0; // middle term

    // result = z2 * BASE^(2*half) + z1 * BASE^half + z0
    BigInt result = shiftLimbs(z2, 2 * half);
    result += shiftLimbs(z1, half);
    result += z0;
    return result;
}

BigInt &operator*=(BigInt &a, const BigInt &b) {
    a = BigInt::karatsuba(a, b);
    return a;
}
BigInt operator*(const BigInt &a, const BigInt &b) {
    return BigInt::karatsuba(a, b);
}

// ═══════════════════════════════════════════════════════════════════════════
//  Division and Modulo — long division on limbs
// ═══════════════════════════════════════════════════════════════════════════

void BigInt::divmod(const BigInt &a, const BigInt &b,
                    BigInt &quot, BigInt &rem) {
    if (b.isZero()) throw domain_error("BigInt: division by zero");
    if (a < b) { quot = BigInt(0ULL); rem = a; return; }
    if (a == b) { quot = BigInt(1ULL); rem = BigInt(0ULL); return; }

    int n = a.numLimbs();
    quot.limbs.assign(n, 0);
    rem = BigInt(0ULL);

    int64_t bTop = b.limbs.back();
    int     bLen = b.numLimbs();

    for (int i = n - 1; i >= 0; i--) {
        rem.limbs.insert(rem.limbs.begin(), a.limbs[i]);
        rem.trim();

        if (rem < b) { quot.limbs[i] = 0; continue; }

        int remLen = rem.numLimbs();
        int64_t rTop;
        if (remLen > bLen)
            rTop = (int64_t)rem.limbs[remLen-1] * BASE + rem.limbs[remLen-2];
        else
            rTop = rem.limbs[remLen-1];

        int64_t qEst = min(rTop / bTop, (int64_t)(BASE - 1));
        BigInt qb = b * BigInt((unsigned long long)qEst);
        while (qb > rem) { --qEst; qb -= b; }

        quot.limbs[i] = (int32_t)qEst;
        rem -= qb;
    }
    quot.trim();
    rem.trim();
}

BigInt &operator/=(BigInt &a, const BigInt &b) {
    BigInt q, r;
    BigInt::divmod(a, b, q, r);
    a = move(q);
    return a;
}
BigInt operator/(const BigInt &a, const BigInt &b) {
    BigInt t(a); t /= b; return t;
}

BigInt &operator%=(BigInt &a, const BigInt &b) {
    BigInt q, r;
    BigInt::divmod(a, b, q, r);
    a = move(r);
    return a;
}
BigInt operator%(const BigInt &a, const BigInt &b) {
    BigInt t(a); t %= b; return t;
}

// ═══════════════════════════════════════════════════════════════════════════
//  Binary Exponentiation  a ^ b  (operator^)
// ═══════════════════════════════════════════════════════════════════════════

BigInt &operator^=(BigInt &base, const BigInt &exp) {
    BigInt result(1ULL);
    BigInt b(base), e(exp);
    while (!e.isZero()) {
        if (e.limbs[0] & 1)   // least-significant bit of e
            result *= b;
        b *= b;
        BigInt::divide_by_2(e);
    }
    base = move(result);
    return base;
}
BigInt operator^(const BigInt &a, const BigInt &b) {
    BigInt t(a); t ^= b; return t;
}

// ═══════════════════════════════════════════════════════════════════════════
//  Integer Square Root  (binary search)
// ═══════════════════════════════════════════════════════════════════════════

BigInt isqrt(const BigInt &a) {
    if (a.isZero()) return BigInt(0ULL);
    BigInt left(1ULL), right(a), best(0ULL);
    BigInt::divide_by_2(right);            // start from a/2
    while (left <= right) {
        BigInt mid = (left + right);
        BigInt::divide_by_2(mid);
        BigInt sq = mid * mid;
        if (sq <= a) { best = mid; left = mid; ++left; }
        else          { right = mid; --right; }
    }
    return best;
}

// ═══════════════════════════════════════════════════════════════════════════
//  GCD / LCM  (binary GCD — avoids expensive modulo for large numbers)
// ═══════════════════════════════════════════════════════════════════════════

BigInt gcd(const BigInt &a, const BigInt &b) {
    if (a.isZero()) return b;
    if (b.isZero()) return a;
    // Euclidean GCD
    BigInt x(a), y(b);
    while (!y.isZero()) { x %= y; swap(x, y); }
    return x;
}

BigInt lcm(const BigInt &a, const BigInt &b) {
    if (a.isZero() || b.isZero()) return BigInt(0ULL);
    return (a / gcd(a, b)) * b;
}

// ═══════════════════════════════════════════════════════════════════════════
//  Combinatorics
// ═══════════════════════════════════════════════════════════════════════════

BigInt Factorial(int n) {
    BigInt f(1ULL);
    for (int i = 2; i <= n; i++) f *= BigInt((unsigned long long)i);
    return f;
}

BigInt NthFibonacci(int n) {
    if (n == 0) return BigInt(0ULL);
    BigInt a(1ULL), b(1ULL), c;
    for (int i = 2; i < n; i++) { c = a + b; a = b; b = c; }
    return (n == 1) ? BigInt(1ULL) : b;
}

BigInt NthCatalan(int n) {
    // C(n) = (2n)! / ((n+1)! * n!)
    BigInt num(1ULL), den(1ULL);
    for (int i = n + 2; i <= 2 * n; i++) num *= BigInt((unsigned long long)i);
    for (int i = 2;     i <= n;     i++) den *= BigInt((unsigned long long)i);
    return num / den;
}

// ═══════════════════════════════════════════════════════════════════════════
//  I/O
// ═══════════════════════════════════════════════════════════════════════════

ostream &operator<<(ostream &os, const BigInt &a) {
    // print most-significant limb without leading zeros, rest with padding
    os << a.limbs.back();
    for (int i = (int)a.limbs.size() - 2; i >= 0; i--) {
        // pad to BASE_DIGITS digits
        string s = to_string(a.limbs[i]);
        os << string(BigInt::BASE_DIGITS - (int)s.size(), '0') << s;
    }
    return os;
}

istream &operator>>(istream &is, BigInt &a) {
    string s; is >> s;
    a = BigInt(s);
    return is;
}
