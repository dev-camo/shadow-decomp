#include "uart_console_io_gcn.h"
#include "UART.h"

int __write_console(__file_handle handle, unsigned char *buffer, size_t *count,
                    __idle_proc idle_fn) {
    static int initialized;

    /* TDEV hardware sends console output through MetroTRK only */
    if ((OSGetConsoleType() & OS_CONSOLE_TDEV) == 0) {
        int uart_init = 0;
        if (initialized == 0) {
            uart_init = InitializeUART(kBaud57600);

            if (uart_init == 0) {
                initialized = 1;
            }
        }

        if (uart_init != 0) {
            return 1;
        }

        if (WriteUARTN(buffer, *count) != 0) {
            *count = 0;
            return 1;
        }
    }

    __TRK_write_console(handle, buffer, count, idle_fn);
    return 0;
}

int __close_console(__file_handle handle) {
    return 0;
}
