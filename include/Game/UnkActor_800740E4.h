#ifndef GAME_UNKACTOR_800740E4_H
#define GAME_UNKACTOR_800740E4_H

#include "boost/weak_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

class UnkManager_8006D6A8;
class UnkManager_80109D64;

/* A polymorphic object that the Chao and Cream gadgets look up through fn_800740E4 when they are
 * first touched (placeholder name and layout: only the virtual method at index 7 and the word at
 * 0xC are used). Minimum size: the size check covers only the members used so far. */
class UnkActor_800740E4 {
  public:
    virtual void UnkSlot0() = 0;
    virtual void UnkSlot1() = 0;
    virtual void UnkSlot2() = 0;
    virtual void UnkSlot3() = 0;
    virtual void UnkSlot4() = 0;
    virtual void UnkSlot5() = 0;
    virtual void UnkSlot6() = 0;
    /* Called with 1 and 0 (arguments unknown). */
    virtual void UnkSlot7(int arg0, int arg1) = 0;

    /* Whether the word at 0xC equals value (name inferred; the gadgets test it against 1, its
     * meaning is unknown). Called as an if condition (possibly negated), it leaves the smart
     * pointer's operator-> out of line, as in the original; a plain field access expands it in
     * place. */
    bool Field0xCEquals(int value) {
        return field_0xC == value;
    }

    /* 0x04 */ u8 field_0x4[0xC - 0x4];
    /* 0x0C */ int field_0xC;
};
CHECK_SIZEOF(UnkActor_800740E4, 0x10);
CHECK_OFFSETOF(UnkActor_800740E4, field_0xC, 0xC);

extern "C" {
/* Singleton managers (constructed on first use, as fn_8001C4F8 does) and calls on them whose
 * names and meaning are unknown (behaviour inferred from the code around the calls). */
UnkManager_8006D6A8 *fn_8006D6A8(void);
boost::weak_ptr<UnkActor_800740E4> fn_800740E4(UnkManager_8006D6A8 *manager, int id);
UnkManager_80109D64 *fn_80109D64(void);
/* Returns its argument. */
int fn_801FAAC4(int arg0);
void fn_801FAF00(UnkManager_80109D64 *manager, int id, int arg2, int arg3, int arg4);
}

#endif
