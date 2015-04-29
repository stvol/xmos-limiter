#include <math.h>
#include <stdint.h>
#include <stdio.h>

#include "fixed_point_functions.h"

//------------------------------------------------------------------------------

int16_t float_to_fixed16(double input, int fractional_digits, int line, char *file)
{
    int16_t i;
    double x;

    if (fractional_digits < 0 || fractional_digits > 16)
    {
        if (file)
            printf("%s\n", file);

        printf("line %d: fractional_digits=%d\n\n", line, fractional_digits);
        return 0;
    }

    // conversion to fractional number
    x = input * pow(2., fractional_digits);

    // overflow detection
    if (x > INT16_MAX)
    {
        if (line > 0)
        {
            if (file)
                printf("%s\n", file);

            printf("line %d: fract16 overflow input=%g, format=%d.%d (%g dB)\n\n",
                    line, input, 16-fractional_digits, fractional_digits,
                    20.*log10(x / INT16_MAX));
        }

        return(INT16_MAX);
    }

    if (x < INT16_MIN)
    {
        if (line > 0)
        {
            if (file)
                printf("%s\n", file);

            printf("line %d: fract16 overflow input=%g, format=%d.%d (%g dB)\n\n",
                    line, input, 16-fractional_digits, fractional_digits,
                    20.*log10(x / INT16_MIN));
        }

        return(INT16_MIN);
    }

    // rounding to 16 bit
    if (x >= 0)
        i = (int16_t) (x + 0.5);
    else
        i = (int16_t) (x - 0.5);

    return i;
}

//------------------------------------------------------------------------------

double fixed16_to_float(int16_t input, int fractional_digits)
{
    return (input * pow(2., -fractional_digits));
}

//------------------------------------------------------------------------------

int32_t float_to_fixed32(double input, int fractional_digits, int line, char *file)
{
    int32_t i;
    double x;

    if (fractional_digits < 0 || fractional_digits > 32)
    {
        if (file)
            printf("%s\n", file);

        printf("line %d: fractional_digits=%d\n\n", line, fractional_digits);
        return 0;
    }

    // conversion to fractional number
    x = input * pow(2., fractional_digits);

    // overflow detection
    if (x > INT32_MAX)
    {
        if (line > 0)
        {
            if (file) printf("%s\n", file);
            printf("line %d: int32 overflow input=%g, format=%d.%d (%g dB)\n\n",
                    line, input, 32-fractional_digits, fractional_digits,
                    20.*log10(x / INT32_MAX));
        }

        return(INT32_MAX);
    }

    if (x < INT32_MIN)
    {
        if (line > 0)
        {
            if (file) printf("%s\n", file);
            printf("line %d: int32 overflow input=%g, format=%d.%d (%g dB)\n\n",
                    line, input, 32-fractional_digits, fractional_digits,
                    20.*log10(x / INT32_MIN));
        }

        return(INT32_MIN);
    }

    // rounding to 32 bit
    if (x >= 0)
        i = (int32_t) (x + 0.5);
    else
        i = (int32_t) (x - 0.5);

    return i;
}

//------------------------------------------------------------------------------

double fixed32_to_float(int32_t input, int fractional_digits)
{
    return (input * pow(2., -fractional_digits));
}

//------------------------------------------------------------------------------

int16_t udiv16(uint16_t x, uint16_t y)
{
    // 16 Bit Division z = x / y
    // x, y, z: Q1.15

    int16_t  z;         // Q1.15
    uint32_t temp32;    // Q2.30

    if (y > x)
    {
        temp32 = ((uint32_t) x) << 15;  // Q1.15  <<  15 -> Q2.30
        z = (int16_t) (temp32 / y);     // Q2.30 / Q1.15 -> Q1.15
    }
    else
    {
        z = 0x7FFF;                     // return ~1 if x >= y
    }

    return z;
}

//------------------------------------------------------------------------------

int32_t udiv32(uint32_t x, uint32_t y)
{
    // 32 Bit Division z = x / y
    // x, y, z: Q1.31

    int32_t z;          // Q1.31
    uint64_t temp64;    // Q2.62

    if (y > x)
    {
        temp64 = ((uint64_t) x) << 31;  // Q1.31  <<  31 -> Q2.62
        z = (int32_t) (temp64 / y);     // Q2.62 / Q1.31 -> Q1.31
    }
    else
    {
        z = 0x7FFFFFFF;                 // return ~1 if x >= y
    }

    return z;
}

//------------------------------------------------------------------------------

inline int nlz(uint32_t x)
{
    // Henry S. Warren, "Hacker's Delight (2nd Edition)", Addison-Wesley, 2012.
    // number of leading zeros, binary search, p. 99.

    int n;

    if (x == 0) return 32;
    n = 0;
    if (x <= 0x0000FFFF) {n = n +16; x = x <<16;}
    if (x <= 0x00FFFFFF) {n = n + 8; x = x << 8;}
    if (x <= 0x0FFFFFFF) {n = n + 4; x = x << 4;}
    if (x <= 0x3FFFFFFF) {n = n + 2; x = x << 2;}
    if (x <= 0x7FFFFFFF) {n = n + 1;}
    return n;
}

//------------------------------------------------------------------------------

int32_t log32(int32_t in_lin, int32_t conv_coeff)
{
    // log-Funktion. Input in_lin im 1.31-Format, Output out_log im 16.16-Format.
    // coeffs sind die Koeffizienten der Taylorreihe im 2.14 Format.
    // conv_coeff im 4.24 Format dient zur Umrechnung in log, log10 und dB.
    // (c) Hagen Jaeger, Uwe Simmer, April 2014.

    static int16_t coeffs[] = {23637, -11819, 7879, -5909, 4727,
                               -3940, 3377, -2955, 2626, -2364};

    int   taylor_cnt, shift_cnt, taylor_deg = 10;
    int16_t product, x1;
    int32_t  out_log;

    in_lin = labs(in_lin);      // sichergehen, dass der Input positiv ist

    shift_cnt = nlz(in_lin)-1;
    in_lin = in_lin << shift_cnt;

    out_log = -shift_cnt << 16; // Umwandlung in 16.16

    x1 = in_lin >> 16;
    x1 = x1 - 0x7FFF;           // (x-1)

    product = x1;   // 1.15-Format, Potenzierung von (x-1) initialisieren
    // Produkt liegt zwischen -1 und 0 (*2^15), 0x7FFF = 1 im 1.15-Format

    // Taylorreihe für log(x)
    for (taylor_cnt = 0; taylor_cnt < taylor_deg; taylor_cnt++)
    {
        out_log += ((int32_t) product * coeffs[taylor_cnt]) >> 13;
        // 1.15 Format mal 2.14 = 3.29 Format, 3.29 >> 13 = 16.16 Format

        product = ((int32_t) product * x1) >> 15;
        // Potenzierung für Taylorreihe: x, x^2, x^3, u.s.w.
        // 1.15 Format mal 1.15 = 2.30, 2.30 >> 15 = 1.15 Format
    }

    // Multiplikation von out_log im 16.16 Format und conv_coeff im 8.24 Format
    out_log = ((int64_t) out_log * conv_coeff) >> 24;

    return out_log;
}

//------------------------------------------------------------------------------

int32_t exp32(int32_t in_log, int32_t conv_coeff)
{
    // e^x-Funktion. Input in_log im 16.16-Format, Output out_lin im 1.31-Format.
    // coeffs sind die Koeffizienten der Taylorreihe ab 3. Ordnung im 0.16 Format.
    // conv_coeff im 8.24 Format dient zur Umrechnung von log, log10 und dB.
    // (c) Hagen Jaeger, Uwe Simmer, April 2014.

    static int16_t coeffs[] = {10923, 2731, 546};
    int shift_cnt, taylor_cnt, taylor_deg = 3;
    int16_t product, x, hw;
    uint16_t lw;
    int16_t ln2 = 0x58B9;
    int32_t temp32;
    uint32_t out_lin;

    // Multiplikation von in_log im 16.16 Format und conv_coeff im 8.24 Format
    in_log = ((int64_t) in_log * conv_coeff) >> 24;

    hw = in_log >> 16;          // extrahieren der oberen 16 Bit, 1.15 Format
    lw = in_log & 0xFFFF;       // extrahieren der unteren 16 Bit, 0.16 Format

    shift_cnt = -hw;            // Anzahl der shifts nach rechts

    if (shift_cnt > 31)
        return 0;

    x = lw >> 1;                // Umwandlung in das 1.15 Format

    out_lin = 0x7FFFFFFF;           // out = 1
    temp32 = ((int32_t) x * ln2);   // x' = x*ln(2)
    out_lin += temp32 << 1;         // out = 1 + x'
    x = (int16_t) (temp32 >> 15);   // x': 1.15
    temp32 = ((int32_t) x * x);     // x' * x'
    out_lin += temp32;              // out = 1 + x' + x'*x'/2
    product = (int16_t) (temp32 >> 15);

    // Taylorreihe für exp(x) ab 3. Ordnung
    for (taylor_cnt = 0; taylor_cnt < taylor_deg; taylor_cnt++)
    {
        product = (int16_t) (((int32_t) product * x) >> 15);
        // Potenzierung für Taylorreihe: x, x^2, x^3, u.s.w.
        // 1.15 Format mal 1.15 = 2.30, 2.30 >> 15 = 1.15 Format

        out_lin += (int32_t) product * coeffs[taylor_cnt];
        // 1.15 Format mal 0.16 Format = 1.31 Format
    }

    // out_lin = 2^frac(in_ld) * 2^int(in_ld)
    out_lin = out_lin >> shift_cnt;

    return out_lin;
}
