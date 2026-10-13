#ifndef TRK_MSGHNDLR_H
#define TRK_MSGHNDLR_H

/* Debugger connection state (msghndlr.c). Kept free of OS headers: usr_put.c includes this but must
 * not see OSReport's variadic prototype. */

#ifdef __cplusplus
extern "C" {
#endif

int GetTRKConnected(void);
void SetTRKConnected(int isConnected);

#ifdef __cplusplus
}
#endif

#endif /* TRK_MSGHNDLR_H */
