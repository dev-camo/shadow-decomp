/* Setup and API layer of Sega's GameCube AX sound library: work area and ARAM setup, the AX frame
 * callback and its frame update, the aux effects, and the checked entry points for players and
 * bank file loads. The original file name is unknown. Definitions are in reverse address order
 * (deferred inlining). Error code names are inferred from where they are returned. */
#include "Game/gcaxSnd.h"
#include <stdarg.h>
#include <string.h>

#define GCAX_ERR_PARAM 0x03000001
#define GCAX_ERR_NO_HANDLE 0x07000101  /* no free player or load request */
#define GCAX_ERR_BAD_HANDLE 0x07000102 /* not an active player or load request */
#define GCAX_ERR_NULL_HANDLE 0x07000103
#define GCAX_ERR_FILE 0x09000001 /* not a bank list file */
#define GCAX_ERR_LOAD 0x09000101
#define GCAX_ERR_NOT_READY 0x0D000001 /* the library is not set up */

typedef void (*GcaxAuxCallback)(void *data, void *context);

/* (defined in reverse address order, like the functions) */
static s32 lbl_805F1408; /* the caller supplied the work area */
GcaxWork *lbl_805F1404;
static void *lbl_805F1400; /* the work area allocation */

/* 0 for an active player, else an error code */
static inline s32 gcaxSndCheckPlayer(GcaxPlayer *player) {
    int i;

    if (player == NULL) {
        return GCAX_ERR_NULL_HANDLE;
    }
    for (i = 0; i < GCAX_PLAYERS; i++) {
        if (player == &lbl_805F1404->players[i]) {
            if (player->flags == 0) {
                break;
            }
            return 0;
        }
    }
    return GCAX_ERR_BAD_HANDLE;
}

/* 0 for a load request in use, else an error code */
static inline s32 gcaxSndCheckLoad(GcaxLoad *load) {
    int i;

    if (load == NULL) {
        return GCAX_ERR_NULL_HANDLE;
    }
    for (i = 0; i < GCAX_LOADS; i++) {
        if (load == &lbl_805F1404->loads[i]) {
            if (load->used == 0) {
                break;
            }
            return 0;
        }
    }
    return GCAX_ERR_BAD_HANDLE;
}

/* Sets up the library with the default work area, voice and ARAM sizes */
s32 fn_8043FB24(void *buffer) {
    fn_8043E6F4(buffer, 48, 0xE1000, 0x800000);
    return 0;
}

/* 0 selects the mono mix, 1 the stereo mix */
s32 fn_8043FAB0(s32 mode) {
    if (lbl_805F1404 == NULL) {
        return GCAX_ERR_NOT_READY;
    }
    switch (mode) {
    case 0:
        fn_8043E47C(1);
        break;
    case 1:
        fn_8043E47C(0);
        break;
    default:
        return GCAX_ERR_PARAM;
    }
    return 0;
}

/* Takes a free load request */
s32 fn_8043FA20(GcaxLoad **out) {
    GcaxLoad *load;
    int i;

    if (lbl_805F1404 == NULL) {
        return GCAX_ERR_NOT_READY;
    }
    load = lbl_805F1404->loads;
    for (i = 0; i < GCAX_LOADS; i++, load++) {
        if (load->used == 0) {
            goto found;
        }
    }
    return GCAX_ERR_NO_HANDLE;

found:
    memset(load, 0, sizeof(GcaxLoad));
    load->used = 1;
    *out = load;
    return 0;
}

/* Sets the file and completion callback of a load request */
s32 fn_8043F980(GcaxLoad *load, u8 *file, u32 arg2, GcaxCallback callback, void *arg) {
    s32 result;

    if (lbl_805F1404 == NULL) {
        return GCAX_ERR_NOT_READY;
    }
    result = gcaxSndCheckLoad(load);
    if (result != 0) {
        return result;
    }
    load->file = file;
    load->request = -1;
    load->field_0x8 = arg2;
    load->callback = callback;
    load->arg = arg;
    return 0;
}

/* Frees a load request, first waiting for its ARAM request. The original reads the busy flag
 * once into a local, so the wait never sees it clear. */
s32 fn_8043F8C4(GcaxLoad *load) {
    s32 result;

    if (lbl_805F1404 == NULL) {
        return GCAX_ERR_NOT_READY;
    }
    result = gcaxSndCheckLoad(load);
    if (result != 0) {
        return result;
    }
    if (load->used && load->request >= 0) {
        volatile u8 *flag = &lbl_805F1404->requestBusy[load->request];
        u8 busy = *flag;

        while (busy != 0) {
        }
    }
    load->used = 0;
    return 0;
}

/* Loads the banks of a bank list file: each 16-byte entry gives a kind (1: wave and parameter
 * data, 4: sound data), a bank number and the offset of its data after the 0x20-byte header. */
/* Loads one bank list entry; negative on failure */
static inline s32 gcaxSndLoadEntry(GcaxLoad *load, u32 *entry, u8 *data) {
    s32 bank = (entry[1] >> 24) & 0xF;
    s32 status = -1;

    switch ((entry[0] >> 24) & 0xF) {
    case 1:
        status =
            fn_8043B05C(bank, data + entry[2], load->callback, load->arg, lbl_805F1404->loadAsync);
        break;
    case 4:
        status = fn_8043AF64(bank, (GcaxFileHeader *)(data + entry[2]), load->callback, load->arg,
                             lbl_805F1404->loadAsync);
        break;
    }
    return status;
}

s32 fn_8043F70C(GcaxLoad *load) {
    GcaxFileHeader *header;
    u8 *data;
    s32 count;
    u32 *entry;
    s32 result;

    if (lbl_805F1404 == NULL) {
        return GCAX_ERR_NOT_READY;
    }
    result = gcaxSndCheckLoad(load);
    if (result != 0) {
        return result;
    }
    header = (GcaxFileHeader *)load->file;
    entry = (u32 *)(header + 2);
    data = (u8 *)entry;
    if (header[0].magic != GCAX_MAGIC || header[0].kind != GCAX_KIND_MLT ||
        header[1].magic != GCAX_MAGIC || header[1].kind != GCAX_KIND_MLTM) {
        return GCAX_ERR_FILE;
    }
    result = 0;
    count = header[1].size >> 4;
    while (count-- > 0) {
        /* an unused entry; the original does not advance past it */
        if (entry[1] == 0xFFFFFFFF) {
            continue;
        }
        if (gcaxSndLoadEntry(load, entry, data) < 0) {
            result = GCAX_ERR_LOAD;
            break;
        }
        entry += 4;
    }
    return result;
}

/* Takes a free player */
s32 fn_8043F6AC(GcaxPlayer **out) {
    GcaxPlayer *player;

    if (lbl_805F1404 == NULL) {
        return GCAX_ERR_NOT_READY;
    }
    player = fn_8043E0E8();
    if (player == NULL) {
        return GCAX_ERR_NO_HANDLE;
    }
    *out = player;
    return 0;
}

/* Frees a player */
s32 fn_8043F60C(GcaxPlayer *player) {
    s32 result;

    if (lbl_805F1404 == NULL) {
        return GCAX_ERR_NOT_READY;
    }
    result = gcaxSndCheckPlayer(player);
    if (result != 0) {
        return result;
    }
    fn_8043E088(player);
    return 0;
}

/* Frees all players. The original walks 64 player slots; the calls past the eight players fail
 * the handle check. */
s32 fn_8043F5AC(void) {
    int i;
    GcaxPlayer *player;

    if (lbl_805F1404 == NULL) {
        return GCAX_ERR_NOT_READY;
    }
    player = lbl_805F1404->players;
    for (i = 0; i < 64; i++, player++) {
        fn_8043F60C(player);
    }
    return 0;
}

/* Builds a MIDI message: status, the first data byte and, for messages with two data bytes, the
 * second one */
s32 fn_8043F41C(GcaxPlayer *player, u8 *msg, u16 status, ...) {
    va_list args;
    s32 result;

    if (lbl_805F1404 == NULL) {
        return GCAX_ERR_NOT_READY;
    }
    result = gcaxSndCheckPlayer(player);
    if (result != 0) {
        return result;
    }
    va_start(args, status);
    msg[0] = status;
    msg[1] = va_arg(args, int);
    switch (status & 0xF0) {
    case 0x80:
    case 0x90:
    case 0xA0:
    case 0xB0:
    case 0xD0:
    case 0xE0:
        msg[2] = va_arg(args, int);
        break;
    default:
        msg[2] = 0;
        break;
    }
    msg[3] = 0;
    va_end(args);
    return 0;
}

/* Queues a MIDI message built by fn_8043F41C with a note priority */
s32 fn_8043F35C(GcaxPlayer *player, u8 *msg, u8 priority) {
    s32 result;

    if (lbl_805F1404 == NULL) {
        return GCAX_ERR_NOT_READY;
    }
    result = gcaxSndCheckPlayer(player);
    if (result != 0) {
        return result;
    }
    fn_8043E004(player, (msg[0] << 24) | (msg[1] << 16) | (msg[2] << 8) | priority);
    return 0;
}

/* Runs a frame update outside the AX callback */
s32 fn_8043F2F0(void) {
    if (lbl_805F1404 == NULL) {
        return GCAX_ERR_NOT_READY;
    }
    gcaxSndLock();
    fn_8043E494();
    gcaxSndUnlock();
    return 0;
}

/* Runs a frame update outside the AX callback */
s32 fn_8043F284(void) {
    if (lbl_805F1404 == NULL) {
        return GCAX_ERR_NOT_READY;
    }
    gcaxSndLock();
    fn_8043E494();
    gcaxSndUnlock();
    return 0;
}

/* Puts the high-quality reverb on aux bus A */
s32 fn_8043F240(void) {
    if (lbl_805F1404 == NULL) {
        return GCAX_ERR_NOT_READY;
    }
    fn_8043E2B4(0, 2);
    return 0;
}

/* Removes the effect from aux bus A */
s32 fn_8043F1FC(void) {
    if (lbl_805F1404 == NULL) {
        return GCAX_ERR_NOT_READY;
    }
    fn_8043E2B4(0, 0);
    return 0;
}

/* Resets all channels of a player */
s32 fn_8043F13C(GcaxPlayer *player) {
    s32 result;
    int i;

    if (lbl_805F1404 == NULL) {
        return GCAX_ERR_NOT_READY;
    }
    result = gcaxSndCheckPlayer(player);
    if (result != 0) {
        return result;
    }
    for (i = 0; i < GCAX_CHANNELS; i++) {
        fn_8043E228(player, i);
    }
    return 0;
}

/* Resets all channels of all players */
s32 fn_8043F0D4(void) {
    int i;

    if (lbl_805F1404 == NULL) {
        return GCAX_ERR_NOT_READY;
    }
    for (i = 0; i < GCAX_PLAYERS; i++) {
        fn_8043F13C(&lbl_805F1404->players[i]);
    }
    return 0;
}

/* Starts song song of the sound data of bank bank with note priority priority */
s32 fn_8043EFF0(GcaxPlayer *player, s8 bank, s8 song, s8 priority) {
    s32 result;

    if (lbl_805F1404 == NULL) {
        return GCAX_ERR_NOT_READY;
    }
    result = gcaxSndCheckPlayer(player);
    if (result != 0) {
        return result;
    }
    if (bank < 0 || bank > 31) {
        return GCAX_ERR_PARAM;
    }
    if (song < 0 || song > 127) {
        return GCAX_ERR_PARAM;
    }
    fn_8043FDF8(player, bank, song, priority);
    return 0;
}

/* Stops the player's sequence */
s32 fn_8043EF50(GcaxPlayer *player) {
    s32 result;

    if (lbl_805F1404 == NULL) {
        return GCAX_ERR_NOT_READY;
    }
    result = gcaxSndCheckPlayer(player);
    if (result != 0) {
        return result;
    }
    fn_8043FD70(player);
    return 0;
}

s32 fn_8043EF48(void) {
    return 0;
}

/* Fades the player's pitch to a number of semitones over time milliseconds */
s32 fn_8043EE74(GcaxPlayer *player, s16 semitones, s32 time) {
    s32 result;

    if (lbl_805F1404 == NULL) {
        return GCAX_ERR_NOT_READY;
    }
    result = gcaxSndCheckPlayer(player);
    if (result != 0) {
        return result;
    }
    fn_8043D7B4(player, fn_8043D318(semitones), time);
    return 0;
}

/* Fades the player's volume to volume / 127 over time milliseconds */
s32 fn_8043ED7C(GcaxPlayer *player, s8 volume, s32 time) {
    s32 result;

    if (lbl_805F1404 == NULL) {
        return GCAX_ERR_NOT_READY;
    }
    result = gcaxSndCheckPlayer(player);
    if (result != 0) {
        return result;
    }
    if (volume < -127 || volume > 127) {
        return GCAX_ERR_PARAM;
    }
    fn_8043D8E0(player, volume / 127.0f, time);
    return 0;
}

/* Pauses a player */
s32 fn_8043ECDC(GcaxPlayer *player) {
    s32 result;

    if (lbl_805F1404 == NULL) {
        return GCAX_ERR_NOT_READY;
    }
    result = gcaxSndCheckPlayer(player);
    if (result != 0) {
        return result;
    }
    fn_8043FC98(player);
    return 0;
}

/* Resumes a paused player */
s32 fn_8043EC3C(GcaxPlayer *player) {
    s32 result;

    if (lbl_805F1404 == NULL) {
        return GCAX_ERR_NOT_READY;
    }
    result = gcaxSndCheckPlayer(player);
    if (result != 0) {
        return result;
    }
    fn_8043FB58(player);
    return 0;
}

/* Frees all players and shuts the library down */
s32 fn_8043EBFC(void) {
    if (lbl_805F1404 == NULL) {
        return GCAX_ERR_NOT_READY;
    }
    fn_8043F5AC();
    fn_8043E624();
    return 0;
}

/* The caller's buffer for the work area, or a new allocation (NULL when none is left) */
static inline void *gcaxSndGetBuffer(void *buffer, u32 size) {
    if (buffer != NULL) {
        lbl_805F1408 = 1;
        return buffer;
    }
    return OSAllocFromHeap(__OSCurrHeap, size);
}

/* Allocates the work area (unless the caller supplies one), puts the sound heap after it and sets
 * the default aux effect parameters */
void fn_8043E9C4(void *buffer, u32 size) {
    BOOL enabled = OSDisableInterrupts();
    u8 *heap;

    buffer = gcaxSndGetBuffer(buffer, size);
    if (buffer == NULL) {
        OSReport("AX SDLIB: Panic: cannot allocate memory (%d)\n", size);
        for (;;) {
        }
    }
    OSReport("AX SDLIB: allocated (%d)\n", size);
    lbl_805F1400 = buffer;
    lbl_805F1404 = (GcaxWork *)(((u32)buffer + 31) & ~31);
    lbl_805F1404->active = 0;
    lbl_805F1404->inCallback = 0;
    OSRestoreInterrupts(enabled);

    memset(lbl_805F1404, 0, sizeof(GcaxWork));
    heap = (u8 *)(((u32)lbl_805F1404 + sizeof(GcaxWork) + 31) & ~31);
    fn_8043AEBC(heap, size - (heap - (u8 *)buffer));

    lbl_805F1404->reverbStd.tempDisableFX = 0;
    lbl_805F1404->reverbStd.time = 3.0f;
    lbl_805F1404->reverbStd.preDelay = 0.1f;
    lbl_805F1404->reverbStd.damping = 0.5f;
    lbl_805F1404->reverbStd.coloration = 0.5f;
    lbl_805F1404->reverbStd.mix = 0.5f;
    lbl_805F1404->reverbHi.tempDisableFX = 0;
    lbl_805F1404->reverbHi.time = 3.0f;
    lbl_805F1404->reverbHi.preDelay = 0.1f;
    lbl_805F1404->reverbHi.damping = 0.5f;
    lbl_805F1404->reverbHi.coloration = 0.5f;
    lbl_805F1404->reverbHi.crosstalk = 0.3f;
    lbl_805F1404->reverbHi.mix = 0.5f;
    lbl_805F1404->chorus.baseDelay = 15;
    lbl_805F1404->chorus.variation = 0;
    lbl_805F1404->chorus.period = 500;
    lbl_805F1404->delay.delay[0] = 500;
    lbl_805F1404->delay.delay[1] = 500;
    lbl_805F1404->delay.delay[2] = 500;
    lbl_805F1404->delay.feedback[0] = 50;
    lbl_805F1404->delay.feedback[1] = 50;
    lbl_805F1404->delay.feedback[2] = 50;
    lbl_805F1404->delay.output[0] = 100;
    lbl_805F1404->delay.output[1] = 100;
    lbl_805F1404->delay.output[2] = 100;
}

/* Sets up the voices and the aux effects */
void fn_8043E950(void) {
    fn_8043D61C();
    AXFXSetHooks(gcaxSndMalloc, fn_8043ACE8);
    AXFXReverbStdInit(&lbl_805F1404->reverbStd);
    AXFXReverbHiInit(&lbl_805F1404->reverbHi);
    AXFXChorusInit(&lbl_805F1404->chorus);
    AXFXDelayInit(&lbl_805F1404->delay);
}

/* Reserves ARAM for every bank with a configured size */
static inline void gcaxSndNewBanks(void) {
    int i;

    for (i = 0; i < GCAX_BANKS; i++) {
        if (lbl_805F1404->config[i] != 0) {
            gcaxNewAramBank(i, (lbl_805F1404->config[i] + 31) & ~31);
        }
    }
}

/* Sets up the library: the work area and sound heap in size bytes of buffer (allocated when
 * NULL), maxVoices voices and aramSize bytes of ARAM, starting with 0x500 bytes of silence and
 * the default ARAM banks */
s32 fn_8043E6F4(void *buffer, s32 maxVoices, u32 size, u32 aramSize) {
    u8 *silence;

    fn_8043E9C4(buffer, size);
    lbl_805F1404->size = sizeof(GcaxWork);
    lbl_805F1404->field_0x2C = 0.0f;
    lbl_805F1404->arqOwner = 15;
    lbl_805F1404->maxVoices = maxVoices;
    lbl_805F1404->aramSize = aramSize;
    lbl_805F1404->aramBase = ARAlloc(lbl_805F1404->aramSize);
    lbl_805F1404->aramNext = (lbl_805F1404->aramBase + 31) & ~31;
    lbl_805F1404->aramRest =
        lbl_805F1404->aramSize - (lbl_805F1404->aramNext - lbl_805F1404->aramBase);

    silence = (u8 *)(((u32)lbl_805F1404->silence + 31) & ~31);
    memset(silence, 0, 0x500);
    fn_8043AB6C((u32)silence, lbl_805F1404->aramNext, 0x500, NULL, NULL);
    lbl_805F1404->silenceAddr = lbl_805F1404->aramNext;
    lbl_805F1404->aramNext += 0x500;
    lbl_805F1404->aramRest -= 0x500;

    lbl_805F1404->field_0x13A44 = 1.0f;
    lbl_805F1404->config[0] = 0x7D000;
    lbl_805F1404->config[1] = 0x704E0;
    lbl_805F1404->config[2] = 0x57E40;
    lbl_805F1404->config[3] = 0x61A80;
    lbl_805F1404->config[4] = 0x57E40;
    lbl_805F1404->config[5] = 0x57E40;
    lbl_805F1404->config[6] = 0x222E0;
    lbl_805F1404->config[7] = 0x186A0;
    lbl_805F1404->config[8] = 0x29820;
    lbl_805F1404->config[9] = 0x30D40;
    lbl_805F1404->config[10] = 0x3A980;
    lbl_805F1404->config[11] = 0x5DC0;
    lbl_805F1404->config[12] = 0x61A80;
    lbl_805F1404->config[13] = 0x9C40;
    fn_8043B25C();
    gcaxSndNewBanks();
    OSReport("AX SDLIB: aramRest=%d\n", lbl_805F1404->aramRest);
    lbl_805F1404->field_0x44 = 1;
    fn_8043E950();
    lbl_805F1404->active = 1;
    return 0;
}

/* Shuts the library down: stops the frame updates and the aux effects and frees the work area
 * (unless the caller supplied it) and the ARAM */
void fn_8043E624(void) {
    u32 length;

    if (lbl_805F1404 != NULL) {
        BOOL enabled = OSDisableInterrupts();

        lbl_805F1404->active = 0;
        lbl_805F1404->lock++;
        OSRestoreInterrupts(enabled);
    }
    AXFXReverbStdShutdown(&lbl_805F1404->reverbStd);
    AXFXReverbHiShutdown(&lbl_805F1404->reverbHi);
    AXFXChorusShutdown(&lbl_805F1404->chorus);
    AXFXDelayShutdown(&lbl_805F1404->delay);
    if (lbl_805F1404 != NULL) {
        lbl_805F1404 = NULL;
        if (lbl_805F1408 != 0) {
            lbl_805F1408 = 0;
        } else {
            OSFreeToHeap(__OSCurrHeap, lbl_805F1400);
        }
        lbl_805F1400 = NULL;
        ARFree(&length);
    }
}

/* The frame update: player messages, fades, voice updates and finished notes of every player */
void fn_8043E494(void) {
    int i;

    if (lbl_805F1404 == NULL || lbl_805F1404->active == 0) {
        return;
    }
    gcaxSndLock();
    if (lbl_805F1404->lock != 1) {
        lbl_805F1404->lockedFrames++;
        gcaxSndUnlock();
        return;
    }
    lbl_805F1404->prevTime = lbl_805F1404->time;
    lbl_805F1404->time = OSGetTime();
    lbl_805F1404->numNotes = fn_8043BB18();
    lbl_805F1404->numVoices = fn_8043D49C();
    for (i = 0; i < GCAX_PLAYERS; i++) {
        if (lbl_805F1404->players[i].flags & 0x80000000) {
            fn_8043FF5C(&lbl_805F1404->players[i]);
            fn_8043DD14(&lbl_805F1404->players[i]);
        }
        fn_8043D99C(&lbl_805F1404->players[i]);
        fn_8043D870(&lbl_805F1404->players[i]);
        fn_8043DAB0(&lbl_805F1404->players[i]);
        fn_8043DA0C(&lbl_805F1404->players[i]);
        fn_8043B5FC(&lbl_805F1404->players[i].notes);
    }
    lbl_805F1404->numNotes = fn_8043BB18();
    lbl_805F1404->numVoices = fn_8043D49C();
    gcaxSndUnlock();
}

void fn_8043E47C(s32 mono) {
    lbl_805F1404->monoMix = mono != 0;
}

/* Sets the AX mode: 0 stereo, else Dolby Pro Logic II */
void fn_8043E430(s32 surround) {
    lbl_805F1404->auxMix = surround != 0;
    if (surround) {
        AXSetMode(2);
    } else {
        AXSetMode(0);
    }
}

/* The AX frame callback. With inCallback set, the frame update runs with interrupts enabled. */
void fn_8043E394(void) {
    if (lbl_805F1404 != NULL && lbl_805F1404->inCallback) {
        lbl_805F1404->callbackCount++;
        OSEnableInterrupts();
        if (lbl_805F1404 != NULL && lbl_805F1404->active != 0) {
            fn_8043E494();
            lbl_805F1404->frames++;
        }
        OSDisableInterrupts();
        MIXUpdateSettings();
    } else {
        if (lbl_805F1404 != NULL && lbl_805F1404->active != 0) {
            fn_8043E494();
        }
        MIXUpdateSettings();
    }
}

/* Puts an effect on an aux bus (0: A, 1: B): 0 none, 1 standard reverb, 2 high-quality reverb,
 * 3 chorus, 4 delay */
void fn_8043E2B4(s32 bus, s32 effect) {
    GcaxAuxCallback callback;
    void *context;

    switch (effect) {
    default:
        return;
    case 0:
        callback = NULL;
        context = NULL;
        break;
    case 1:
        callback = (GcaxAuxCallback)AXFXReverbStdCallback;
        context = &lbl_805F1404->reverbStd;
        break;
    case 2:
        callback = (GcaxAuxCallback)AXFXReverbHiCallback;
        context = &lbl_805F1404->reverbHi;
        break;
    case 3:
        callback = (GcaxAuxCallback)AXFXChorusCallback;
        context = &lbl_805F1404->chorus;
        break;
    case 4:
        callback = (GcaxAuxCallback)AXFXDelayCallback;
        context = &lbl_805F1404->delay;
        break;
    }
    switch (bus) {
    case 0:
        AXRegisterAuxACallback(callback, context);
        break;
    case 1:
        AXRegisterAuxBCallback(callback, context);
        break;
    }
}
