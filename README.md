# BigInt — Arbitrary-Precision Integer Library (C++)

A header + implementation library for arbitrary-precision arithmetic in C++, built without any third-party dependencies. Designed to bypass the 64-bit integer limit with native-like operator syntax, optimised memory layout, and sub-quadratic multiplication.

---

## Features

- **Base-10⁹ limb storage** using `std::vector<int32_t>` — ~9× more space-efficient than base-10, with better cache locality
- **Karatsuba multiplication** — O(n^1.585) vs naive O(n²), with automatic schoolbook fallback for small inputs
- **Binary exponentiation** via `operator^` — computes `a^b` in O(log b) multiplications
- **Full operator overloading** — use BigInts like native integers (`+`, `-`, `*`, `/`, `%`, `^`, `==`, `<`, `++`, etc.)
- **Integer square root**, **GCD**, **LCM**
- **Combinatorics** — Factorial, Nth Fibonacci, Nth Catalan number
- Stream I/O via `cin` / `cout`

---

## File Structure

```
BigInt.h      # Class declaration, all operator signatures, constants
BigInt.cpp    # Full implementation (Karatsuba, division, exponentiation, etc.)
main.cpp      # Demo and correctness tests for all features
```

---

## Build & Run

Requires C++17 and any standard compiler (GCC / Clang / MSVC).

```bash
g++ -O2 -std=c++17 -o demo BigInt.cpp main.cpp
./demo
```

---

## Quick Start

```cpp
#include "BigInt.h"

// Construction
BigInt a("123456789012345678901234567890");
BigInt b(9876543210ULL);
BigInt c("999999999999999999999999999999");

// Arithmetic — works exactly like built-in integers
BigInt sum     = a + b;
BigInt product = a * b;
BigInt quotient= a / b;
BigInt rem     = a % b;

// Exponentiation (binary fast-power)
BigInt base(2ULL), exp(100ULL);
BigInt result = base ^ exp;   // 2^100

// Comparison
if (a > b) { /* ... */ }
if (a == c) { /* ... */ }

// Increment / Decrement
++a;
b--;

// I/O
std::cout << a << "\n";
std::cin  >> a;

// Utility functions
BigInt root    = isqrt(a);          // integer square root
BigInt g       = gcd(a, b);
BigInt l       = lcm(a, b);

// Combinatorics
BigInt f       = Factorial(100);
BigInt fib     = NthFibonacci(200);
BigInt cat     = NthCatalan(15);

// Inspect
std::cout << a.numDigits() << "\n"; // total decimal digits
std::cout << a.numLimbs()  << "\n"; // internal limb count
```

---

## API Reference

### Constructors

| Constructor | Description |
|---|---|
| `BigInt()` | Initialises to 0 |
| `BigInt(unsigned long long n)` | From a 64-bit unsigned integer |
| `BigInt(long long n)` | From a 64-bit signed integer (non-negative) |
| `BigInt(const std::string &s)` | From a decimal string |
| `BigInt(const char *s)` | From a C-string |

### Arithmetic Operators

| Operator | Description |
|---|---|
| `+`, `+=` | Addition |
| `-`, `-=` | Subtraction (throws on underflow) |
| `*`, `*=` | Multiplication (Karatsuba) |
| `/`, `/=` | Integer division |
| `%`, `%=` | Modulo |
| `^`, `^=` | Exponentiation via binary fast-power |
| `++`, `--` | Pre/post increment and decrement |

### Comparison Operators

`==`, `!=`, `<`, `<=`, `>`, `>=`

### Utility Functions

| Function | Signature | Description |
|---|---|---|
| Integer square root | `isqrt(const BigInt &a)` | Floor of √a |
| GCD | `gcd(const BigInt &a, const BigInt &b)` | Greatest common divisor |
| LCM | `lcm(const BigInt &a, const BigInt &b)` | Least common multiple |
| Factorial | `Factorial(int n)` | n! |
| Fibonacci | `NthFibonacci(int n)` | F(n), 0-indexed |
| Catalan | `NthCatalan(int n)` | C(n) |

### Member Helpers

| Method | Description |
|---|---|
| `numDigits()` | Total number of decimal digits |
| `numLimbs()` | Number of internal base-10⁹ limbs |
| `isZero()` | Returns `true` if value is 0 |
| `operator[](int idx)` | Returns the idx-th decimal digit (0 = least significant) |

---

## Design Details

### Storage: Base-10⁹ Limbs

Numbers are stored little-endian in a `std::vector<int32_t>`, where each element (a "limb") holds a value in `[0, 10⁹)`. This packs 9 decimal digits per 4-byte integer, compared to 1 digit per byte in a naive string-based approach — a 9× reduction in memory and proportional improvement in cache utilisation.

```
Value:  123456789012345678
Limbs:  [12345678, 123456789]   // [least-significant, most-significant]
         └─ limbs[0]  └─ limbs[1]
```

### Multiplication: Karatsuba Algorithm

For large inputs (≥ `KARA_THRESHOLD = 32` limbs), multiplication uses the Karatsuba algorithm instead of the O(n²) schoolbook method.

Given two n-limb numbers A and B, split each at the midpoint:

```
A = A₁ · BASE^m  +  A₀
B = B₁ · BASE^m  +  B₀

A × B = z₂ · BASE^(2m)  +  z₁ · BASE^m  +  z₀

where:
  z₂ = A₁ × B₁
  z₀ = A₀ × B₀
  z₁ = (A₁ + A₀)(B₁ + B₀) − z₂ − z₀   ← only 3 recursive multiplications
```

This reduces the number of recursive sub-multiplications from 4 (schoolbook) to 3, yielding complexity O(n^log₂3) ≈ O(n^1.585).

### Exponentiation: Binary Fast-Power

`operator^` implements the standard square-and-multiply algorithm, performing O(log b) multiplications to compute `a^b`:

```cpp
result = 1
while exp > 0:
    if exp is odd: result *= base
    base *= base
    exp  /= 2        // divide_by_2() on BigInt
```

---

## Complexity Summary

| Operation | Complexity |
|---|---|
| Addition / Subtraction | O(n) |
| Multiplication (Karatsuba) | O(n^1.585) |
| Division | O(n²) |
| Exponentiation (`a^b`) | O(log b · M(n)) where M(n) = Karatsuba cost |
| Integer square root | O(log(a) · M(n)) |
| GCD (Euclidean) | O(log(min(a,b)) · M(n)) |

n = number of limbs in the operand.

---

## Limitations

- Only **non-negative integers** are supported (no sign bit).
- Division is correct but uses a limb-by-limb long-division approach; for very large divisors, Newton-Raphson division would be faster.
- The `operator^` symbol is used for exponentiation (not bitwise XOR, which is its standard C++ meaning). Keep this in mind when reading expressions.
