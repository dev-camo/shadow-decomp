#ifndef GAME_UNKACTOR_800740E4_H
#define GAME_UNKACTOR_800740E4_H

#include "boost/weak_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

class UnkManager_8006D6A8;
class UnkManager_80109D64;

/* A polymorphic object that gadgets look up through fn_800740E4 (placeholder name and layout:
 * only the virtual method at index 7, the word at 0xC and the byte at 0x14 are used). Minimum
 * size: the size check covers only the members used so far. */
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

    /* Matching workaround: calling these inline accessors through the smart pointer (Field0xCEquals
     * as an if condition, possibly negated) leaves the smart pointer's operator-> out of line, as
     * in the original; a plain field access expands it in place. The original accessors are
     * unknown. Revisit when they are identified.
     *
     * Whether the word at 0xC equals value (name inferred; the gadgets test it against 1 and 2,
     * its meaning is unknown). */
    bool Field0xCEquals(int value) {
        return field_0xC == value;
    }

    /* Stores value in the byte at 0x14 (name inferred; its meaning is unknown). */
    void SetField0x14(u8 value) {
        field_0x14 = value;
    }

    /* 0x04 */ u8 field_0x4[0xC - 0x4];
    /* 0x0C */ int field_0xC;
    /* 0x10 */ u8 field_0x10[0x14 - 0x10];
    /* 0x14 */ u8 field_0x14; /* meaning unknown; a gadget stores 1 here */
};
CHECK_SIZEOF(UnkActor_800740E4, 0x18);
CHECK_OFFSETOF(UnkActor_800740E4, field_0xC, 0xC);
CHECK_OFFSETOF(UnkActor_800740E4, field_0x14, 0x14);

extern "C" {
/* Singleton managers (constructed on first use, as fn_8001C4F8 does) and calls on them whose
 * names and meaning are unknown (behaviour inferred from the code around the calls). */
UnkManager_8006D6A8 *fn_8006D6A8(void);
boost::weak_ptr<UnkActor_800740E4> fn_800740E4(UnkManager_8006D6A8 *manager, int id);
UnkManager_80109D64 *fn_80109D64(void);
/* Returns its argument. */
int fn_801FAAC4(int arg0);
/* Returns -1 at once when the manager's word at 0x24 is non-zero. Otherwise it locks the manager's
 * weak pointer; when that gives an object, it passes the object, id, arg2 and 0 to fn_80049E1C and
 * returns -1 without queueing when that returns null or the word at 0x8 of the entry it returns
 * is less than the manager's word at 0x28. When it does not return -1 (also when the weak pointer
 * gives no object), it calls fn_801FABE4, queues one record {id, arg2, index} for each index from
 * arg4 up to 35 and returns 1000 times id plus 100 times arg2. arg3 is not read (behaviour
 * inferred from its body; the HINTRING gadget keeps the result and passes it to fn_801FAAC8, the
 * other callers ignore it). */
int fn_801FAF00(UnkManager_80109D64 *manager, int id, int arg2, int arg3, int arg4);
/* For an id of 0 or more: false when id equals 1000 times the manager's word at 0x14 plus 100
 * times its word at 0x18, or the same sum of one of its queued records, and true otherwise. For a
 * negative id: true when fn_801FABD4 returns false (behaviour inferred from its body). */
bool fn_801FAAC8(UnkManager_80109D64 *manager, int id);
/* Stores 1 in the actor's word at 0xC and calls fn_8016B058 with the actor (behaviour inferred
 * from its body). */
void fn_8016B2A4(UnkActor_800740E4 *actor);
/* Passes an actor of the manager to fn_8016B2A4 and returns true, or returns false when none
 * qualifies (behaviour inferred from its body; the selection by the actors' state is not
 * understood). The GoalRing gadget calls it when a character touches the ring. */
bool fn_8016DA34(UnkManager_8006D6A8 *manager);
}

#endif
