#ifndef _VA_LIST_STRUCT_H_
#define _VA_LIST_STRUCT_H_

/* The PowerPC EABI variable argument record, shared by the C library facade (stdarg.h) and MSL
 * (va_list), and the runtime function that fetches the next argument (Runtime/Src/__va_arg.c). */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __va_list_struct {
    char gpr; /* general registers used */
    char fpr; /* floating-point registers used */
    char reserved[2];
    char *input_arg_area; /* the arguments passed on the stack */
    char *reg_save_area;  /* the argument registers saved by the callee */
} _va_list_struct;

typedef _va_list_struct __va_list[1];

/* type is the argument's class from _var_arg_typeof */
void *__va_arg(_va_list_struct *list, int type);

#ifdef __cplusplus
}
#endif

#endif
