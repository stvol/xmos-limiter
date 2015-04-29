#ifndef _FIXED_POINT_FUNCTIONS
#define _FIXED_POINT_FUNCTIONS

#include <stdint.h>
#include <stdlib.h>

// 16-bit multiplication with quantization
inline int16_t mul16(int16_t x, int16_t y, const int fractional_digits)
{
    return (int16_t) (((int32_t) x * y) >> fractional_digits);
}

// 16-bit multiplication without quantization
inline int32_t mul16(int16_t x, int16_t y)
{
    return ((int32_t) x * y);
}

// 16-bit quantization
inline int16_t quantization16(int32_t x, const int fractional_digits)
{
    return (int16_t) (x >> fractional_digits);
}

// 32-bit by 16-bit multiplication
inline int32_t mult32x16(int32_t x, int16_t y)
{
    // 1.31 by 1.15 multiplication
    // 1.31 result

    int32_t z;

    // (x.l * y.h) >> 16
    z = ((int32_t) y * (uint16_t) x) >> 15;

    // (x.h * y.h)
    z += ((int32_t) (x >> 16) * y) << 1;

    return z;
}

// 32-bit by 16-bit unsigned multiplication
inline int32_t mult32u16(int32_t x, uint16_t y)
{
    // 1.31 by 0.16 multiplication
    // 1.31 result

    int32_t z;

    // (x.l * y.h) >> 16
    z = ((int32_t) y * (uint16_t) x) >> 16;

    // (x.h * y.h)
    z += ((int32_t) (x >> 16) * y);

    return z;
}

// 32-bit multiplication with quantization
inline int32_t mul32(int32_t x, int32_t y, const int fractional_digits)
{
    return (int32_t) (((int64_t) x * y) >> fractional_digits);
}

// 32-bit multiplication without quantization
inline int64_t mul32(int32_t x, int32_t y)
{
    return ((int64_t) x * y);
}

// 32-bit quantization
inline int32_t quantization32(int64_t x, const int fractional_digits)
{
    return (int32_t) (x >> fractional_digits);
}

// 16-bit float to fixed-point conversion
int16_t float_to_fixed16(double input, int fractional_digits, int line=-1, char *file=0);

// 16-bit fixed-point to float conversion
double fixed16_to_float(int16_t input, int fractional_digits);

// 32-bit float to fixed-point conversion
int32_t float_to_fixed32(double input, int fractional_digits, int line=-1, char *file=0);

// 32-bit fixed-point to float conversion
double fixed32_to_float(int32_t input, int fractional_digits);

// 16-bit unsigned division
int16_t udiv16(uint16_t x, uint16_t y);

// 32-bit unsigned division
int32_t udiv32(uint32_t x, uint32_t y);

// 32-bit logarithm
int32_t log32(int32_t in_lin, int32_t conv_coeff);

// 32-bit exponential function
int32_t exp32(int32_t in_log, int32_t conv_coeff);

#define L_DB    0x06054609
#define L_LOG10 0x004D104D
#define L_LOG   0x00B17218
#define L_LD    0x01000000

#define E_DB    0x002A854B
#define E_LOG10 0x035269E1
#define E_LOG   0x01715476
#define E_LD    0x01000000

#ifndef max
#define max(a, b) (a > b ? a : b)
#endif
#ifndef min
#define min(a, b) (a < b ? a : b)
#endif

#endif
