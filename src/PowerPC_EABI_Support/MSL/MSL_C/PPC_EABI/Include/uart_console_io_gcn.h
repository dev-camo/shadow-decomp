#ifndef PPC_EABI_SRC_UART_CONSOLE_IO_GCN_H
#define PPC_EABI_SRC_UART_CONSOLE_IO_GCN_H

#include "ansi_files.h"

#ifdef __cplusplus
extern "C" {
#endif

/* From <dolphin/os.h>, which cannot be included with the MSL include paths: dolphin/types.h pulls
 * in the game's libc facade headers, which need <stdarg.h>, <stddef.h> and cmath.h from
 * include/libc. */
#define OS_CONSOLE_TDEV 0x20000000
unsigned long OSGetConsoleType(void);

/* MetroTRK's console output */
int __TRK_write_console(__file_handle handle, unsigned char *buffer, size_t *count,
                        __idle_proc idle_fn);

#ifdef __cplusplus
}
#endif

#endif /* PPC_EABI_SRC_UART_CONSOLE_IO_GCN_H */
