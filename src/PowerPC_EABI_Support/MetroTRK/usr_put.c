#include <dolphin/types.h>
#include "MetroTRK/msghndlr.h"

// No OS header is included: the original OSReport call has no crclr, so OSReport's variadic
// prototype was not in scope here.

BOOL usr_puts_serial(const char *msg) {
    BOOL connect_ = FALSE;
    char c;
    char buf[2];

    while (!connect_ && (c = *msg++) != '\0') {
        BOOL connect = GetTRKConnected();

        buf[0] = c;
        buf[1] = '\0';

        SetTRKConnected(FALSE);
        OSReport(buf);

        SetTRKConnected(connect);
        connect_ = FALSE;
    }
    return connect_;
}

void usr_put_initialize(void) {}
