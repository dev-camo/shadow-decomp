#ifndef METROTRK_MEMMAP_H
#define METROTRK_MEMMAP_H
#include <dolphin/types.h>
#include "layout_check.h"

typedef struct memRange {
    /* 0x0 */ u8 *start;
    /* 0x4 */ u8 *end;
    /* 0x8 */ BOOL readable;
    /* 0xC */ BOOL writeable;
} memRange;
CHECK_SIZEOF(memRange, 0x10);

extern const memRange gTRKMemMap[1];

#endif
