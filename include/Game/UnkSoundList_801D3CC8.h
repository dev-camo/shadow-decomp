#ifndef GAME_UNKSOUNDLIST_801D3CC8_H
#define GAME_UNKSOUNDLIST_801D3CC8_H

#include <dolphin/types.h>
#include "layout_check.h"

class UnkSoundHandle_801D3DC8;

/* The nontrivial list member constructed at 0x801D452C (placeholder name). Its links can refer
 * to an allocated node or its in-place end marker; their common pointee type is unrecovered. */
class UnkSoundListStorage_801D452C {
  public:
    UnkSoundListStorage_801D452C();
    ~UnkSoundListStorage_801D452C();

    /* 0x00 */ u32 field_0x0;
    /* 0x04 */ void *field_0x4;
    /* 0x08 */ void *field_0x8;

  private:
    /* Copying unrecovered. */
    UnkSoundListStorage_801D452C(const UnkSoundListStorage_801D452C &other);
    UnkSoundListStorage_801D452C &operator=(const UnkSoundListStorage_801D452C &other);
};
CHECK_SIZEOF(UnkSoundListStorage_801D452C, 0xC);

/* The singleton returned by fn_801D3CC8 (placeholder name), constructed at 0x801D44E4 and
 * destroyed at 0x801D4384. The separate AudioLoopManagerTask visits its stored handles. */
class UnkSoundList_801D3CC8 {
  public:
    UnkSoundList_801D3CC8();
    ~UnkSoundList_801D3CC8();

    /* Removes the first record with the handle's owner, id and serial (behaviour inferred).
     * The const-reference spelling is inferred from read-only use of the caller's handle. */
    void fn_801D4110(const UnkSoundHandle_801D3DC8 &handle);

    /* 0x00 */ u16 field_0x0;
    /* 0x04 */ UnkSoundListStorage_801D452C field_0x4;
    /* 0x10 */ u32 field_0x10;

  private:
    /* Copying unrecovered. */
    UnkSoundList_801D3CC8(const UnkSoundList_801D3CC8 &other);
    UnkSoundList_801D3CC8 &operator=(const UnkSoundList_801D3CC8 &other);
};
CHECK_SIZEOF(UnkSoundList_801D3CC8, 0x14);

extern "C" UnkSoundList_801D3CC8 *fn_801D3CC8(void);

#endif
