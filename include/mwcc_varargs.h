#ifndef MWCC_VARARGS_H
#define MWCC_VARARGS_H

/* Compiler EABI cursor view used by __builtin_va_info.
 * The original source typedef identity is unknown.
 */
typedef struct {
    /* 0x00 */ unsigned char gpr;
    /* 0x01 */ unsigned char fpr;
    /* 0x02 */ unsigned char reserved[2];
    /* 0x04 */ void *input_arg_area;
    /* 0x08 */ void *reg_save_area;
} MwccVaList[1];

#ifdef __cplusplus
extern "C" {
#endif

/* MSL vsprintf; its va_list parameter is spelled with this cursor view. */
extern int vsprintf(char *buffer, const char *format, MwccVaList args);

#ifdef __cplusplus
}
#endif

#endif
