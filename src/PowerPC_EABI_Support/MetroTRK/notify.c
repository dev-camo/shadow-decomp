#include "MetroTRK/trk.h"

DSError TRKDoNotifyStopped(MessageCommandID cmd) {
    int reqIdx;
    int bufIdx;
    MessageBuffer *msg;
    DSError err;

    err = TRKGetFreeBuffer(&bufIdx, &msg);
    if (err == DS_NoError) {
        // The same test again: a single test changes the code (the original has two consecutive
        // branches on the result).
        if (err == DS_NoError) {
            if (cmd == DSMSG_NotifyStopped) {
                TRKTargetAddStopInfo(msg);
            } else {
                TRKTargetAddExceptionInfo(msg);
            }
        }
        err = TRKRequestSend(msg, &reqIdx, 2, 3, 1);
        if (err == DS_NoError) {
            TRKReleaseBuffer(reqIdx);
        }
        TRKReleaseBuffer(bufIdx);
    }
    return err;
}
