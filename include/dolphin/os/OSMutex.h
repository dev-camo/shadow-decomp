#ifndef _DOLPHIN_OSMUTEX_H_
#define _DOLPHIN_OSMUTEX_H_

#include <dolphin/os/OSThread.h>
#include "layout_check.h"

#ifdef __cplusplus
extern "C" {
#endif

struct OSMutex {
    /* 0x00 */ OSThreadQueue queue;
    /* 0x08 */ OSThread *thread;
    /* 0x0C */ s32 count;
    /* 0x10 */ OSMutexLink link;
};
CHECK_SIZEOF(struct OSMutex, 0x18);

typedef struct OSCond {
    OSThreadQueue queue;
} OSCond;
CHECK_SIZEOF(OSCond, 0x8);

void OSInitMutex(OSMutex *mutex);
void OSLockMutex(OSMutex *mutex);
void OSUnlockMutex(OSMutex *mutex);
BOOL OSTryLockMutex(OSMutex *mutex);
void OSInitCond(OSCond *cond);
void OSWaitCond(OSCond *cond, OSMutex *mutex);
void OSSignalCond(OSCond *cond);

#ifdef __cplusplus
}
#endif

#endif
