#pragma once
#include <bits/stdc++.h>

/*
 * BigInt — Arbitrary-Precision Integer Library
 * Storage  : Base-10^9 in std::vector<int32_t> (little-endian limbs)
 *            Each limb holds a value in [0, 10^9).
 *            This maximises cache locality and cuts space by ~9× vs base-10.
 * Multiply : Karatsuba algorithm  O(n^log2(3)) ≈ O(n^1.585)
 *            Falls back to schoolbook O(n^2) for small inputs (< KARA_THRESHOLD limbs).
 * Power    : Binary exponentiation (fast-power) via operator^
 */

class BigInt {
public:
    // ── internal constants ──────────────────────────────────────────────────
    static constexpr int32_t BASE      = 1'000'000'000; // 10^9
    static constexpr int32_t BASE_DIGITS = 9;
    static constexpr int      KARA_THRESHOLD = 32;      // limbs; tune as needed

    // ── storage ─────────────────────────────────────────────────────────────
    // limbs[0] is the least-significant limb (little-endian).
    std::vector<int32_t> limbs;

    // ── constructors ────────────────────────────────────────────────────────
    BigInt();                           // value 0
    explicit BigInt(long long n);
    BigInt(unsigned long long n);
    explicit BigInt(const std::string &s);
    explicit BigInt(const char *s);
    BigInt(const BigInt &) = default;
    BigInt(BigInt &&)      = default;

    // ── assignment ──────────────────────────────────────────────────────────
    BigInt &operator=(const BigInt &) = default;
    BigInt &operator=(BigInt &&)      = default;

    // ── helpers ─────────────────────────────────────────────────────────────
    bool   isZero()  const;
    int    numDigits() const;           // total decimal digits
    int    numLimbs()  const { return (int)limbs.size(); }

    // ── increment / decrement ───────────────────────────────────────────────
    BigInt &operator++();
    BigInt  operator++(int);
    BigInt &operator--();
    BigInt  operator--(int);

    // ── subscript (0 = least-significant decimal digit) ─────────────────────
    int operator[](int idx) const;

    // ── arithmetic (binary) ─────────────────────────────────────────────────
    friend BigInt &operator+=(BigInt &a, const BigInt &b);
    friend BigInt &operator-=(BigInt &a, const BigInt &b);
    friend BigInt &operator*=(BigInt &a, const BigInt &b);
    friend BigInt &operator/=(BigInt &a, const BigInt &b);
    friend BigInt &operator%=(BigInt &a, const BigInt &b);
    friend BigInt &operator^=(BigInt &a, const BigInt &b); // fast power

    friend BigInt operator+(const BigInt &a, const BigInt &b);
    friend BigInt operator-(const BigInt &a, const BigInt &b);
    friend BigInt operator*(const BigInt &a, const BigInt &b);
    friend BigInt operator/(const BigInt &a, const BigInt &b);
    friend BigInt operator%(const BigInt &a, const BigInt &b);
    friend BigInt operator^(const BigInt &a, const BigInt &b); // fast power

    // ── comparison ──────────────────────────────────────────────────────────
    friend bool operator==(const BigInt &a, const BigInt &b);
    friend bool operator!=(const BigInt &a, const BigInt &b);
    friend bool operator< (const BigInt &a, const BigInt &b);
    friend bool operator<=(const BigInt &a, const BigInt &b);
    friend bool operator> (const BigInt &a, const BigInt &b);
    friend bool operator>=(const BigInt &a, const BigInt &b);

    // ── I/O ─────────────────────────────────────────────────────────────────
    friend std::ostream &operator<<(std::ostream &os, const BigInt &a);
    friend std::istream &operator>>(std::istream &is,       BigInt &a);

    // ── math utilities ──────────────────────────────────────────────────────
    friend BigInt isqrt(const BigInt &a);        // integer square root
    friend BigInt gcd (const BigInt &a, const BigInt &b);
    friend BigInt lcm (const BigInt &a, const BigInt &b);

    // ── combinatorics ────────────────────────────────────────────────────────
    friend BigInt Factorial   (int n);
    friend BigInt NthFibonacci(int n);
    friend BigInt NthCatalan  (int n);

    void  trim();    // remove leading zero-limbs (kept public so helpers can call it)

private:
    void  fromString(const std::string &s);

    // low-level limb arithmetic (used internally)
    static BigInt  schoolbook_mul(const BigInt &a, const BigInt &b);
    static BigInt  karatsuba     (const BigInt &a, const BigInt &b);
    static void    divmod(const BigInt &a, const BigInt &b,
                          BigInt &quot, BigInt &rem);
    static void    divide_by_2(BigInt &a);   // exact halving
};

// ── free-function declarations (defined in BigInt.cpp) ──────────────────────
BigInt Factorial   (int n);
BigInt NthFibonacci(int n);
BigInt NthCatalan  (int n);
BigInt isqrt(const BigInt &a);
BigInt gcd  (const BigInt &a, const BigInt &b);
BigInt lcm  (const BigInt &a, const BigInt &b);
