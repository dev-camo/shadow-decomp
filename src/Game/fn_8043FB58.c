/* Sequence playback of Sega's GameCube AX sound library: Standard MIDI File tracks from the sound
 * data, played through a player's MIDI event queue, with pause and resume. The original file name
 * is unknown. Definitions are in reverse address order (deferred inlining). */
#include "Game/gcaxSnd.h"
#include <string.h>

/* Reads a MIDI variable-length quantity at *p and moves *p past it; -1 when it is longer than four
 * bytes (then *p stays). */
static inline s32 gcaxSndReadVarLen(u8 **p) {
    s32 more = 0;
    s32 value;
    int i;
    u8 *q;
    u8 *start;

    start = *p;
    q = start;

    for (i = 0; i < 4; i++) {
        if (!(*q & 0x80)) {
            goto found;
        }
        more++;
        q++;
    }
    return -1;

found:
    value = 0;
    for (i = 0; i < more + 1; i++) {
        value = (value << 7) + (*start++ & 0x7F);
    }
    *p = start;
    return value;
}

/* Ends the player's sequence: silences its notes and clears the sequence state */
static inline void gcaxSndEndSequence(GcaxPlayer *player) {
    player->state &= ~GCAX_PLAYER_SEQUENCE;
    if (player->notes.head != NULL) {
        fn_8043B840(&player->notes);
        player->notes.head = NULL;
    }
    memset(&player->seq, 0, sizeof(GcaxSequence));
}

/* Plays the player's sequence up to the current frame time */
void fn_8043FF5C(GcaxPlayer *player) {
    u8 *cursor;
    u8 *next;
    u8 status;
    s32 delta;
    f32 frameTicks;

    if (!(player->flags & GCAX_PLAYER_IN_USE)) {
        return;
    }
    if (!(player->state & GCAX_PLAYER_SEQUENCE)) {
        return;
    }
    if (player->state & GCAX_PLAYER_PAUSED) {
        return;
    }
    cursor = player->seq.cursor;
    if (cursor == NULL) {
        return;
    }
    lbl_805F1404->elapsed = lbl_805F1404->time - player->seq.lastTime;
    if (lbl_805F1404->elapsed < 0) {
        lbl_805F1404->elapsed = -lbl_805F1404->elapsed;
    }

restart:
    player->seq.beatTime = 60.0f / player->seq.tempo;
    frameTicks = OSTicksToMicroseconds((u32)lbl_805F1404->elapsed);
    frameTicks *= lbl_805F1404->field_0x13A44;
    frameTicks *= player->seq.division;
    frameTicks *= player->seq.tempo;
    frameTicks /= 60000000.0f;
    player->seq.prevNow = player->seq.now;
    player->seq.delta = frameTicks;
    player->seq.now += frameTicks;
    do {
        next = cursor;
        delta = gcaxSndReadVarLen(&next);
        if (delta < 0) {
            cursor = NULL;
            break;
        }
        if (player->seq.time + delta > player->seq.now) {
            break;
        }
        cursor = next;
        player->seq.prevTime = player->seq.time;
        player->seq.time += delta;

        if (*cursor & 0x80) {
            status = *cursor++;
        } else {
            status = player->seq.runningStatus;
        }
        if (status == 0xFF) {
            player->seq.runningStatus = 0xFF;
            switch (*cursor) {
            case 0x51:
                /* set tempo: microseconds per quarter note */
                player->seq.tempo =
                    60000000.0f / ((cursor[2] << 16) + (cursor[3] << 8) + cursor[4]);
                cursor += 5;
                break;
            case 0x2F:
                /* end of track */
                cursor = NULL;
                goto done;
            }
        } else {
            switch (status & 0xF0) {
            case 0x80:
            case 0x90:
            case 0xE0:
                fn_8043E004(player, (status << 24) + (cursor[0] << 16) + (cursor[1] << 8) +
                                        (u8)player->seq.priority);
                player->seq.runningStatus = status;
                cursor += 2;
                break;
            case 0xB0:
                if (cursor[0] == 0x1F) {
                    /* controller 0x1F marks a loop: the first one sets the loop point and count (0
                     * loops forever), later ones jump back while the count lasts */
                    if (player->seq.loopStart == NULL) {
                        player->seq.loopStart = cursor + 2;
                        player->seq.loopCount = cursor[1];
                        cursor += 2;
                        player->seq.loopTime = player->seq.time;
                        player->seq.loopNow = player->seq.prevNow;
                        player->seq.loopTempo = player->seq.tempo;
                        break;
                    }
                    if (player->seq.loopCount == 0) {
                    loop:
                        cursor = player->seq.loopStart;
                        player->seq.now = player->seq.loopNow;
                        player->seq.time = player->seq.loopTime;
                        player->seq.tempo = player->seq.loopTempo;
                        goto restart;
                    }
                    if (player->seq.loopCount > 0) {
                        player->seq.loopCount--;
                        if (player->seq.loopCount > 0) {
                            goto loop;
                        }
                        player->seq.runningStatus = 0;
                        cursor += 2;
                    }
                    break;
                }
                fn_8043E004(player, (status << 24) + (cursor[0] << 16) + (cursor[1] << 8) +
                                        (u8)player->seq.priority);
                player->seq.runningStatus = status;
                cursor += 2;
                break;
            case 0xA0:
            case 0xC0:
            case 0xD0:
                fn_8043E004(player, (status << 24) + (cursor[0] << 16));
                player->seq.runningStatus = status;
                cursor += 1;
                break;
            }
        }
    } while (player->seq.time < player->seq.now);

done:
    player->seq.cursor = cursor;
    player->seq.lastTime = lbl_805F1404->time;
    if (cursor == NULL) {
        gcaxSndEndSequence(player);
    }
}

/* Starts song song (the top byte of its entry) of the sound data of bank bank */
s32 fn_8043FDF8(GcaxPlayer *player, s32 bank, s32 song, s32 priority) {
    u32 *table;
    u32 *entry;
    BOOL enabled;
    u8 *file;
    GcaxSequence *seq;

    if (!(player->flags & GCAX_PLAYER_IN_USE)) {
        return -1;
    }
    table = (u32 *)lbl_805F1404->soundData[bank].addr;
    if (table == NULL) {
        return -1;
    }
    for (entry = table; *entry != 0; entry++) {
        if (GCAX_COUNT(*entry) == song) {
            goto found;
        }
    }
    return -1;

found:
    file = (u8 *)table + GCAX_OFFSET(*entry);
    if (((GcaxFileHeader *)file)->magic != GCAX_MAGIC ||
        ((GcaxFileHeader *)file)->kind != GCAX_KIND_MSBM) {
        return -1;
    }
    enabled = OSDisableInterrupts();
    player->state = 0;
    player->state |= GCAX_PLAYER_SEQUENCE;
    if (player->notes.head != NULL) {
        fn_8043B840(&player->notes);
        player->notes.head = NULL;
    }
    seq = &player->seq;
    /* The original reads the entry again here and recomputes file (lwz r0,0(r28) and
     * add r30,r29,r0 at fn_8043FDF8+0xE0, after OSDisableInterrupts and fn_8043B840). */
    file = (u8 *)table + GCAX_OFFSET(*entry);
    /* the original sets this before clearing the whole state */
    seq->field_0x0 = *(u16 *)(file + 0x10);
    memset(seq, 0, sizeof(GcaxSequence));
    seq->division = *(u16 *)(file + 0x1E);
    seq->lastTime = seq->startTime = lbl_805F1404->time;
    seq->tempo = 120.0f;
    seq->priority = priority;
    seq->cursor = seq->start = file + 0x28;
    OSRestoreInterrupts(enabled);
    return 0;
}

/* Stops the player's sequence */
s32 fn_8043FD70(GcaxPlayer *player) {
    BOOL enabled;

    if (!(player->flags & GCAX_PLAYER_IN_USE)) {
        return -1;
    }
    enabled = OSDisableInterrupts();
    gcaxSndEndSequence(player);
    OSRestoreInterrupts(enabled);
    return 0;
}

/* Pauses the player: its sequence stops advancing and its notes stop */
void fn_8043FC98(GcaxPlayer *player) {
    BOOL enabled;

    if (!(player->flags & GCAX_PLAYER_IN_USE)) {
        return;
    }
    enabled = OSDisableInterrupts();
    gcaxSndLock();
    if (!(player->state & GCAX_PLAYER_PAUSED)) {
        player->pauseTime = lbl_805F1404->time;
        player->state |= GCAX_PLAYER_PAUSED;
        if (player->notes.head != NULL) {
            if (lbl_805F1404->field_0x44 == 1) {
                fn_8043B6D0(&player->notes);
            } else {
                fn_8043B840(&player->notes);
                player->notes.head = NULL;
            }
        }
    }
    gcaxSndUnlock();
    OSRestoreInterrupts(enabled);
}

/* Resumes a paused player, moving its sequence times past the pause */
void fn_8043FB58(GcaxPlayer *player) {
    BOOL enabled;

    if (!(player->flags & GCAX_PLAYER_IN_USE)) {
        return;
    }
    enabled = OSDisableInterrupts();
    gcaxSndLock();
    if (!(player->state & GCAX_PLAYER_SEQUENCE)) {
        player->state &= ~GCAX_PLAYER_PAUSED;
        gcaxSndUnlock();
        OSRestoreInterrupts(enabled);
        return;
    }
    if (player->state & GCAX_PLAYER_PAUSED) {
        player->seq.startTime += lbl_805F1404->time - player->pauseTime;
        player->seq.lastTime += lbl_805F1404->time - player->pauseTime;
        player->state &= ~GCAX_PLAYER_PAUSED;
        if (lbl_805F1404->field_0x44 == 1) {
            fn_8043B794(&player->notes);
        }
    }
    gcaxSndUnlock();
    OSRestoreInterrupts(enabled);
}
