#ifndef _MSL_COMMON_ANSI_FP_H
#define _MSL_COMMON_ANSI_FP_H
#include "layout_check.h"

#define SIGDIGLEN 36

typedef struct decimal {
    /* 0x00 */ char sign;
    /* 0x01 */ char field_0x1;
    /* 0x02 */ short exp;
    struct {
        /* 0x04 */ unsigned char length;
        /* 0x05 */ unsigned char text[SIGDIGLEN];
        /* 0x29 */ unsigned char field_0x29;
    } sig;
} decimal;
CHECK_SIZEOF(decimal, 0x2A);

typedef struct decform {
    /* 0x0 */ char style;
    /* 0x1 */ char field_0x1;
    /* 0x2 */ short digits;
} decform;
CHECK_SIZEOF(decform, 0x4);

void __ull2dec(decimal *, unsigned long long);
void __timesdec(decimal *, const decimal *, const decimal *);
void __str2dec(decimal *, const char *, short);
void __two_exp(decimal *, long);
int __equals_dec(const decimal *, const decimal *);
int __less_dec(const decimal *, const decimal *);
void __minus_dec(decimal *, const decimal *, const decimal *);
void __num2dec_internal(decimal *, double);
void __num2dec(const decform *, double, decimal *);
double __dec2num(const decimal *);

#endif
