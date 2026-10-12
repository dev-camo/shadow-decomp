#ifndef PPC_EABI_UART_H
#define PPC_EABI_UART_H

/* Serial console interface the MSL console driver uses (implemented by the OS EXI UART code). */
#include <cstddef>

#ifdef __cplusplus
extern "C" {
#endif

typedef int UARTError;

typedef enum UARTBaudRate {
    kBaud300 = 300,
    kBaud600 = 600,
    kBaud1200 = 1200,
    kBaud1800 = 1800,
    kBaud2000 = 2000,
    kBaud2400 = 2400,
    kBaud3600 = 3600,
    kBaud4800 = 4800,
    kBaud7200 = 7200,
    kBaud9600 = 9600,
    kBaud19200 = 19200,
    kBaud38400 = 38400,
    kBaud57600 = 57600,
    kBaud115200 = 115200,
    kBaud230400 = 230400
} UARTBaudRate;

UARTError InitializeUART(UARTBaudRate baudRate);
UARTError WriteUARTN(const void *bytes, unsigned long length);

#ifdef __cplusplus
}
#endif

#endif /* PPC_EABI_UART_H */
