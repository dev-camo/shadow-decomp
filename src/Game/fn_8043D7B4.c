/* MIDI sequence players of Sega's GameCube AX sound library: player allocation, the MIDI event
 * queue and its channel messages, and the volume and pitch fades. The original file name is
 * unknown. Definitions are in reverse address order (deferred inlining). */
#include "Game/gcaxSnd.h"
#include <string.h>

/* Restores a channel's controllers to their defaults */
static inline void gcaxSndResetChannel(GcaxPlayer *player, int channel) {
    BOOL enabled = OSDisableInterrupts();

    player->channels[channel].flags = 0;
    player->channels[channel].volume = 100;
    player->channels[channel].pan = 64;
    player->channels[channel].pitchBend = 0x2000;
    player->channels[channel].expression = 127;
    player->channels[channel].field_0x20 = 0;
    player->channels[channel].field_0x28 = 0;
    player->channels[channel].field_0x24 = -1;
    player->channels[channel].field_0x38 = 0x88888888;
    player->channels[channel].field_0x3C = 0x88888888;
    player->channels[channel].field_0x40 = 0x88888888;
    OSRestoreInterrupts(enabled);
}

/* Releases the notes of a key on a channel, unless the channel's sustain pedal is down. A player
 * with flag bit 30 drops the notes at once instead. */
static inline void gcaxSndNoteOff(GcaxPlayer *player, u8 channel, u8 key) {
    GcaxNode *node;

    if (player->channels[channel].flags & 0x1) {
        return;
    }
    for (node = NULL; (node = fn_8043B8F0(&player->notes, node)) != NULL;) {
        GcaxNote *note = node->data;

        if (note == NULL || note->channel != channel || note->key != key) {
            continue;
        }
        if (player->flags & 0x40000000) {
            fn_8043B868(&player->notes, node);
        } else {
            GcaxNode *ref;

            for (ref = NULL; (ref = fn_8043B3DC(&note->voices, ref)) != NULL;) {
                GcaxVoice *voice = ref->data;

                if (voice != NULL && voice->envState < 4) {
                    voice->envState = 4;
                }
            }
        }
    }
}

/* Moves a fade one step: the value follows the fade until its time has run out */
static inline void gcaxSndStepFade(f32 *value, GcaxFade *fade) {
    if (fade->remaining <= 0.0f) {
        *value = fade->target;
        fade->time = 0.0f;
        fade->remaining = -1.0f;
    } else {
        *value = (fade->target - fade->start) * (1.0f - fade->remaining / fade->time) + fade->start;
    }
}

/* The pitch bend relative to its centre */
static inline void gcaxSndUpdateBend(GcaxPlayer *player, int channel) {
    player->channels[channel].pitchBendOffset = player->channels[channel].pitchBend - 0x2000;
}

/* Resets a channel's controllers */
void fn_8043E228(GcaxPlayer *player, int channel) {
    gcaxSndResetChannel(player, channel);
}

/* Takes a free player and sets it up with default channels; NULL when all are in use */
GcaxPlayer *fn_8043E0E8(void) {
    BOOL enabled = OSDisableInterrupts();
    GcaxPlayer *player;
    int i;

    for (i = 0; i < GCAX_PLAYERS; i++) {
        if (!(lbl_805F1404->players[i].flags & 0x80000000)) {
            goto found;
        }
    }
    OSRestoreInterrupts(enabled);
    return NULL;

found:
    player = &lbl_805F1404->players[i];
    memset(player, 0, sizeof(GcaxPlayer));
    player->flags |= 0x80000000;
    player->volume = 0.0f;
    player->field_0xC = 0.0f;
    player->pitchScale = 1.0f;
    player->field_0x28 = 0.0f;
    player->field_0x10 = 1.0f;
    player->volumeFade.remaining = -1.0f;
    player->pitchFade.remaining = -1.0f;
    for (i = 0; i < GCAX_CHANNELS; i++) {
        player->channels[i].field_0x4 = i;
        gcaxSndResetChannel(player, i);
    }
    OSRestoreInterrupts(enabled);
    return player;
}

/* Frees a player and its notes */
void fn_8043E088(GcaxPlayer *player) {
    BOOL enabled = OSDisableInterrupts();

    if (player->notes.head != NULL) {
        fn_8043B840(&player->notes);
        player->notes.head = NULL;
    }
    player->flags &= ~0x80000000;
    OSRestoreInterrupts(enabled);
}

/* Queues a MIDI message; it is dropped when the queue is full */
void fn_8043E004(GcaxPlayer *player, u32 event) {
    BOOL enabled = OSDisableInterrupts();

    if (player->eventCount < GCAX_EVENTS) {
        player->events[player->eventWrite] = event;
        player->eventCount++;
        player->eventWrite++;
        player->eventWrite %= GCAX_EVENTS;
    }
    OSRestoreInterrupts(enabled);
}

/* Handles the queued MIDI messages: note off and on, control and program change, pitch bend */
void fn_8043DD14(GcaxPlayer *player) {
    while (player->eventCount != 0) {
        BOOL enabled = OSDisableInterrupts();
        u32 event;
        u8 channel;
        u8 data1;
        u8 data2;
        int priority;

        player->eventCount--;
        event = player->events[player->eventRead];
        player->eventRead++;
        player->eventRead %= GCAX_EVENTS;
        OSRestoreInterrupts(enabled);

        channel = (event >> 24) & 0xF;
        data1 = (event >> 16) & 0xFF;
        data2 = (event >> 8) & 0xFF;
        priority = (s8)(event & 0xFF);
        switch ((event >> 24) & 0xF0) {
        case 0x80:
            gcaxSndNoteOff(player, channel, data1);
            break;
        case 0x90:
            if (data2 != 0) {
                GcaxNode *node = fn_8043B910(&player->notes, NULL);
                GcaxNote *note;

                if (node == NULL) {
                    break;
                }
                note = fn_8043B954(player, channel, data1, data2, priority);
                if (note == NULL) {
                    fn_8043B868(&player->notes, node);
                    break;
                }
                note->owner = player;
                note->key = data1;
                note->channel = channel;
                node->data = note;
            } else {
                gcaxSndNoteOff(player, channel, data1);
            }
            break;
        case 0xA0:
            break;
        case 0xB0:
            fn_8043DB08(player, channel, data1, data2);
            break;
        case 0xC0:
            player->channels[channel].program = data1 & 0x7F;
            break;
        case 0xD0:
            break;
        case 0xE0:
            fn_8043DAE4(player, channel, ((data2 & 0x7F) << 7) + (data1 & 0x7F));
            break;
        }
    }
}

/* MIDI control change */
void fn_8043DB08(GcaxPlayer *player, u8 channel, u8 control, u8 value) {
    switch (control) {
    case 0x00:
    case 0x20:
        player->channels[channel].bank = value & 0x7F;
        break;
    case 0x07:
        player->channels[channel].volume = value & 0x7F;
        break;
    case 0x0A:
        player->channels[channel].pan = value & 0x7F;
        player->channels[channel].field_0x24 = -1;
        break;
    case 0x0B:
        player->channels[channel].expression = value & 0x7F;
        break;
    case 0x3F:
        player->channels[channel].field_0x20 = value & 0x7F;
        break;
    case 0x3E: {
        s32 send;
        s32 route;

        if ((value & 0x7Fu) <= 64) {
            send = (value & 0x7F) * 2;
            if (send > 127) {
                send = 127;
            }
            route = 0;
        } else {
            route = 127;
            send = (127 - (value & 0x7F)) * 2;
        }
        player->channels[channel].field_0x24 = send;
        player->channels[channel].field_0x20 = route;
        break;
    }
    case 0x40:
        /* sustain pedal */
        if ((value & 0x7Fu) >= 64) {
            player->channels[channel].flags |= 0x1;
        } else {
            player->channels[channel].flags &= ~0x1;
        }
        break;
    case 0x79:
        /* reset all controllers */
        gcaxSndResetChannel(player, channel);
        break;
    }
}

/* MIDI pitch bend: a 14-bit value centred on 0x2000 */
void fn_8043DAE4(GcaxPlayer *player, u8 channel, u16 bend) {
    player->channels[channel].pitchBend = bend & 0x3FFF;
    gcaxSndUpdateBend(player, channel);
}

/* Recomputes every channel's pitch bend offset */
void fn_8043DAB0(GcaxPlayer *player) {
    int i;

    if (!(player->flags & 0x80000000)) {
        return;
    }
    for (i = 0; i < GCAX_CHANNELS; i++) {
        gcaxSndUpdateBend(player, i);
    }
}

/* Updates envelope, mix and pitch of all voices of the player's notes */
void fn_8043DA0C(GcaxPlayer *player) {
    GcaxNode *node;

    if (!(player->flags & 0x80000000)) {
        return;
    }
    for (node = NULL; (node = fn_8043B8F0(&player->notes, node)) != NULL;) {
        GcaxNode *ref;
        int channel = ((GcaxNote *)node->data)->channel;

        for (ref = NULL; (ref = fn_8043C10C(&((GcaxNote *)node->data)->voices, ref)) != NULL;) {
            fn_8043BE6C(ref->data);
            fn_8043C2A4(ref->data, player);
            fn_8043C170(ref->data, player->channels[channel].pitchBendOffset, player->pitchScale);
        }
    }
}

/* Advances the volume fade by one 5 ms tick */
void fn_8043D99C(GcaxPlayer *player) {
    if (player->volumeFade.remaining >= 0.0f) {
        player->volumeFade.remaining -= 5.0f;
        gcaxSndStepFade(&player->volume, &player->volumeFade);
    }
}

/* Starts a volume fade to target over time milliseconds */
void fn_8043D8E0(GcaxPlayer *player, f32 target, f32 time) {
    BOOL enabled = OSDisableInterrupts();

    player->volumeFade.start = player->volume;
    player->volumeFade.target = target;
    player->volumeFade.time = time;
    player->volumeFade.remaining = time;
    gcaxSndStepFade(&player->volume, &player->volumeFade);
    OSRestoreInterrupts(enabled);
}

/* Advances the pitch fade by one 5 ms tick */
void fn_8043D870(GcaxPlayer *player) {
    if (player->pitchFade.remaining >= 0.0f) {
        player->pitchFade.remaining -= 5.0f;
        gcaxSndStepFade(&player->pitchScale, &player->pitchFade);
    }
}

/* Starts a pitch scale fade to target over time milliseconds */
void fn_8043D7B4(GcaxPlayer *player, f32 target, f32 time) {
    BOOL enabled = OSDisableInterrupts();

    player->pitchFade.start = player->pitchScale;
    player->pitchFade.target = target;
    player->pitchFade.time = time;
    player->pitchFade.remaining = time;
    gcaxSndStepFade(&player->pitchScale, &player->pitchFade);
    OSRestoreInterrupts(enabled);
}
