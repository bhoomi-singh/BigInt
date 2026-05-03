#include "BigInt.h"
#include <iostream>
#include <iomanip>
#include <chrono>

using namespace std;
using namespace chrono;

// ─── tiny benchmark helper ───────────────────────────────────────────────────
template<typename F>
double bench_ms(F &&f) {
    auto t0 = high_resolution_clock::now();
    f();
    auto t1 = high_resolution_clock::now();
    return duration<double, milli>(t1 - t0).count();
}

// ─── section banner ──────────────────────────────────────────────────────────
void section(const string &title) {
    cout << "\n" << string(60, '=') << "\n"
         << "  " << title << "\n"
         << string(60, '=') << "\n";
}

int main() {
    cout << "╔══════════════════════════════════════════════════════════╗\n"
         << "║        BigInt  —  Arbitrary-Precision Library Demo       ║\n"
         << "║  Storage : Base-10^9 limbs in std::vector<int32_t>      ║\n"
         << "║  Multiply: Karatsuba  O(n^1.585)  + Binary Exponentiation║\n"
         << "╚══════════════════════════════════════════════════════════╝\n";

    // ── 1. Basic construction & I/O ─────────────────────────────────────────
    section("1. CONSTRUCTION & I/O");
    {
        BigInt a("146823");
        BigInt b(146823ULL);
        BigInt c(9876543210123456789ULL);

        cout << "  From string  : " << a << "\n";
        cout << "  From ull     : " << b << "\n";
        cout << "  Large ull    : " << c << "\n";
        cout << "  Decimal digits in a: " << a.numDigits() << "\n";
        cout << "  Internal limbs  in a: " << a.numLimbs()  << "\n";
    }

    // ── 2. Comparison ───────────────────────────────────────────────────────
    section("2. COMPARISON OPERATORS");
    {
        BigInt x("200000"), y("6000000"), z("90000000");
        cout << "  x=" << x << "  y=" << y << "  z=" << z << "\n";
        cout << "  x == 200000 ? " << (x == BigInt("200000") ? "yes" : "no") << "\n";
        cout << "  x <  y      ? " << (x < y  ? "yes" : "no") << "\n";
        cout << "  z >  y      ? " << (z > y  ? "yes" : "no") << "\n";
        cout << "  y >= y      ? " << (y >= y ? "yes" : "no") << "\n";
    }

    // ── 3. Increment / Decrement ────────────────────────────────────────────
    section("3. INCREMENT / DECREMENT");
    {
        BigInt n("999999999999999999");   // crosses limb boundary
        cout << "  Before ++  : " << n << "\n";
        ++n;
        cout << "  After  ++  : " << n << "\n";
        n--;
        cout << "  After  --  : " << n << "\n";
    }

    // ── 4. Addition & Subtraction ───────────────────────────────────────────
    section("4. ADDITION & SUBTRACTION");
    {
        BigInt a("99999999999999999999999999");
        BigInt b("1");
        cout << "  a          = " << a << "\n";
        cout << "  b          = " << b << "\n";
        cout << "  a + b      = " << (a + b) << "\n";
        cout << "  a - b      = " << (a - b) << "\n";

        BigInt p("6000000"), q("90000000");
        cout << "  p + q      = " << (p + q) << "\n";
    }

    // ── 5. Multiplication — schoolbook vs Karatsuba ─────────────────────────
    section("5. MULTIPLICATION  (Karatsuba O(n^1.585))");
    {
        BigInt a("146823"), b("200000");
        cout << "  " << a << " × " << b << " = " << (a * b) << "\n";

        // Benchmark: multiply two 500-digit numbers
        string s(500, '9');  // 10^500 - 1
        BigInt big(s);

        double ms = bench_ms([&]{ volatile auto r = big * big; (void)r; });
        BigInt result = big * big;
        cout << "\n  Benchmark: (10^500-1)^2\n";
        cout << "  Karatsuba  time : " << fixed << setprecision(2) << ms << " ms\n";
        cout << "  Result digits   : " << result.numDigits() << "\n";
    }

    // ── 6. Division & Modulo ────────────────────────────────────────────────
    section("6. DIVISION & MODULO");
    {
        BigInt a("1000000000000000000000");
        BigInt b("999999937");   // large prime
        cout << "  a          = " << a << "\n";
        cout << "  b          = " << b << "\n";
        cout << "  a / b      = " << (a / b) << "\n";
        cout << "  a % b      = " << (a % b) << "\n";

        // Verify: (a/b)*b + (a%b) == a
        BigInt q = a / b, r = a % b;
        cout << "  Verify (q*b + r == a): "
             << ((q * b + r == a) ? "PASS" : "FAIL") << "\n";
    }

    // ── 7. Binary Exponentiation ────────────────────────────────────────────
    section("7. BINARY EXPONENTIATION  (operator^)");
    {
        BigInt base(2ULL), exp(100ULL);
        cout << "  2^100      = " << (base ^ exp) << "\n";

        BigInt b3(3ULL), e50(50ULL);
        cout << "  3^50       = " << (b3 ^ e50) << "\n";

        // 2^1000
        BigInt b2(2ULL), e1000(1000ULL);
        BigInt r = b2 ^ e1000;
        cout << "  2^1000 (digits=" << r.numDigits() << "): " << r << "\n";
    }

    // ── 8. Integer Square Root ──────────────────────────────────────────────
    section("8. INTEGER SQUARE ROOT");
    {
        BigInt a("152399025");     // 12345^2
        cout << "  isqrt(152399025)    = " << isqrt(a) << "  (expect 12345)\n";

        BigInt b("99999999999999999999");
        cout << "  isqrt(10^20 - 1)    = " << isqrt(b) << "\n";
    }

    // ── 9. GCD & LCM ────────────────────────────────────────────────────────
    section("9. GCD & LCM");
    {
        BigInt a("123456789012345678901234567890");
        BigInt b("987654321098765432109876543210");
        cout << "  a = " << a << "\n";
        cout << "  b = " << b << "\n";
        cout << "  gcd(a,b) = " << gcd(a, b) << "\n";
        cout << "  lcm(a,b) = " << lcm(a, b) << "\n";
    }

    // ── 10. Factorial ────────────────────────────────────────────────────────
    section("10. FACTORIAL  0! … 20! (spot-check), then 100! and 1000!");
    {
        for (int i = 0; i <= 20; i++)
            cout << "  " << setw(2) << i << "! = " << Factorial(i) << "\n";

        BigInt f100 = Factorial(100);
        cout << "\n  100! digits = " << f100.numDigits() << "\n";
        cout << "  100!        = " << f100 << "\n";

        BigInt f1000;
        double ms = bench_ms([&]{ f1000 = Factorial(1000); });
        cout << "\n  1000! digits = " << f1000.numDigits() << "\n";
        cout << "  1000! time   = " << fixed << setprecision(2) << ms << " ms\n";
    }

    // ── 11. Fibonacci ────────────────────────────────────────────────────────
    section("11. FIBONACCI  F(0)…F(20) (spot-check), F(100) & F(1000)");
    {
        for (int i = 0; i <= 20; i++)
            cout << "  F(" << setw(2) << i << ") = " << NthFibonacci(i) << "\n";

        cout << "\n  F(100)        = " << NthFibonacci(100) << "\n";
        cout << "  F(1000) digits= " << NthFibonacci(1000).numDigits() << "\n";
        cout << "  F(1000)       = " << NthFibonacci(1000) << "\n";
    }

    // ── 12. Catalan Numbers ──────────────────────────────────────────────────
    section("12. CATALAN NUMBERS  C(0)…C(15)");
    {
        for (int i = 0; i <= 15; i++)
            cout << "  C(" << setw(2) << i << ") = " << NthCatalan(i) << "\n";
    }

    // ── 13. Stress / correctness check ──────────────────────────────────────
    section("13. CORRECTNESS CROSS-CHECKS");
    {
        // (a+b)*(a-b) == a^2 - b^2
        BigInt a("123456789012345678901"), b("987654321");
        BigInt lhs = (a + b) * (a - b);
        BigInt rhs = a * a - b * b;
        cout << "  (a+b)*(a-b) == a²-b² : " << (lhs == rhs ? "PASS" : "FAIL") << "\n";

        // Verify: 2^10 = 1024
        BigInt two(2ULL), ten(10ULL);
        BigInt pw = two ^ ten;
        cout << "  2^10 = " << pw << "  (expect 1024): "
             << (pw == BigInt(1024ULL) ? "PASS" : "FAIL") << "\n";

        // sqrt(n^2) == n
        BigInt n("9999999999999");
        BigInt sq = n * n;
        cout << "  isqrt(n^2) == n       : "
             << (isqrt(sq) == n ? "PASS" : "FAIL") << "\n";
    }

    cout << "\n" << string(60, '=') << "\n"
         << "  All demos complete.\n"
         << string(60, '=') << "\n";
    return 0;
}
