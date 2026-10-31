#ifndef GAME_UNKSOUNDHANDLE_801D3DC8_H
#define GAME_UNKSOUNDHANDLE_801D3DC8_H

#include "Game/Task.h"
#include "Game/fn_8040_vector_types.h"
#include <dolphin/types.h>
#include "layout_check.h"

/* The sound handle that fn_801D3DC8 and fn_801D3E58 return (placeholder name; the layout comes from
 * the constructor at 0x8007D770, the assignment at 0x8007D624 and the two functions that fill
 * it). The assignment copies the fields one by one and calls fn_800091D0 for the position. */
class UnkSoundHandle_801D3DC8 {
  public:
    UnkSoundHandle_801D3DC8();
    UnkSoundHandle_801D3DC8 &operator=(const UnkSoundHandle_801D3DC8 &other);
    /* Stops the sound and clears the handle (behaviour inferred from its body). */
    void fn_801D455C();

    /* 0x00 */ Task *field_0x0; /* the task that owns the sound */
    /* 0x04 */ u16 field_0x4;   /* the sound id */
    /* 0x06 */ u16 field_0x6;
    /* 0x08 */ u16 field_0x8;
    /* 0x0A */ u8 field_0xA;
    /* 0x0B */ u8 field_0xB;
    /* 0x0C */ Fn8040Vector3 field_0xC;
    /* 0x18 */ const Fn8040Vector3 *field_0x18;
};
CHECK_SIZEOF(UnkSoundHandle_801D3DC8, 0x1C);

extern "C" {
/* Starts the sound id; the first one follows position (the pointer is kept), the second one
 * copies it (behaviour inferred from their bodies). owner is stored in the handle. */
UnkSoundHandle_801D3DC8 fn_801D3DC8(u16 id, const Fn8040Vector3 *position, Task *owner);
UnkSoundHandle_801D3DC8 fn_801D3E58(u16 id, const Fn8040Vector3 *position, Task *owner);
}

#endif
