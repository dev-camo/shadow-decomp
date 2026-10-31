// Dolphin SDK DVD library: loading the file system table at boot.
#include <dolphin.h>
#include <dolphin/os.h>
#include <dolphin/dvd.h>

#include "__dvd.h"

// Large enough for a 32-byte aligned DVDBB2 (OSRoundUp32B moves the start up by at most 31 bytes).
static u8 bb2Buf[sizeof(DVDBB2) + 31];

static u32 status;
static DVDBB2 *bb2;
static DVDDiskID *idTmp;

static void cb(s32 result, DVDCommandBlock *block);

static void cb(s32 result, DVDCommandBlock *block) {
    if (result > 0) {
        switch (status) {
        case 0:
            status = 1;
            DVDReadAbsAsyncForBS(block, bb2, 0x20, 0x420, cb);
            return;
        case 1:
            status = 2;
            DVDReadAbsAsyncForBS(block, bb2->FSTAddress, (bb2->FSTLength + 0x1F) & 0xFFFFFFE0,
                                 bb2->FSTPosition, cb);
        default:
            return;
        }
    }

    if (result == DVD_RESULT_FATAL_ERROR) {
        return;
    } else if (result == -4) {
        status = 0;
        DVDReset();
        DVDReadDiskID(block, idTmp, cb);
    }
}

void __fstLoad(void) {
    OSBootInfo *bootInfo;
    DVDDiskID *id;
    u8 idTmpBuf[sizeof(DVDDiskID) + 31];
    s32 state;
    void *arenaHi;
    static DVDCommandBlock block;

    arenaHi = OSGetArenaHi();
    bootInfo = OSPhysicalToCached(0);
    idTmp = (void *)OSRoundUp32B(idTmpBuf);
    bb2 = (void *)OSRoundUp32B(bb2Buf);

    DVDReset();
    DVDReadDiskID(&block, idTmp, cb);

    while (1) {
        state = DVDGetDriveStatus();
        if (state == DVD_STATE_END) {
            break;
        }
    }

    bootInfo->FSTLocation = (void *)bb2->FSTAddress;
    bootInfo->FSTMaxLength = bb2->FSTMaxLength;
    id = &bootInfo->DVDDiskID;
    memcpy(id, idTmp, 0x20);
    OSReport("\n");
    OSReport("  Game Name ... %c%c%c%c\n", id->gameName[0], id->gameName[1], id->gameName[2],
             id->gameName[3]);
    OSReport("  Company ..... %c%c\n", id->company[0], id->company[1]);
    OSReport("  Disk # ...... %d\n", id->diskNumber);
    OSReport("  Game ver .... %d\n", id->gameVersion);
    OSReport("  Streaming ... %s\n", !(id->streaming) ? "OFF" : "ON");
    OSReport("\n");
    OSSetArenaHi(bb2->FSTAddress);
}
