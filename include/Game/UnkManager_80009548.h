#ifndef GAME_UNKMANAGER_80009548_H
#define GAME_UNKMANAGER_80009548_H

#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

/* The object that fn_80009548 returns: a static object (at 0x8056EE14 in the USA release) that
 * the function constructs and registers for destruction on first use. Its class and the original
 * function names are unknown. */
class UnkManager_80009548;

/* The entry of the manager that fn_8000D0F0 looks up by index (placeholder name; the original class
 * is unknown). Only the two vectors whose addresses fn_80010244 and fn_800101F8 return and the
 * matrix at 0x6C are mapped here; the size is not known. */
struct UnkEntry_8000D0F0 {
    /* 0x00 */ u8 field_0x0[0x8];
    /* 0x08 */ Fn8040Vector3 field_0x8;
    /* 0x14 */ Fn8040Vector3 field_0x14;
    /* 0x20 */ u8 field_0x20[0x4C];
    /* 0x6C */ Fn8040Matrix34 field_0x6C;
};
CHECK_OFFSETOF(UnkEntry_8000D0F0, field_0x8, 0x8);
CHECK_OFFSETOF(UnkEntry_8000D0F0, field_0x14, 0x14);
CHECK_OFFSETOF(UnkEntry_8000D0F0, field_0x6C, 0x6C);

extern "C" {
/* Returns the object, constructing it on first use. */
UnkManager_80009548 *fn_80009548(void);
/* Behaviour inferred from the CityLaser gadget's call only: three floats and an integer (0). */
void fn_8000D664(UnkManager_80009548 *manager, int arg1, float arg2, float arg3, float arg4);
/* Returns a vector that the BDTrue gadget uses as angles (behaviour inferred). */
Fn8040Vector3 fn_80109B80(UnkManager_80009548 *manager, int arg1);
/* Returns the address of the vector 0x8 bytes into the entry with the given index (behaviour
 * inferred from its body). BDHologram and RainEffect pass 0 for index. */
Fn8040Vector3 *fn_80010244(UnkManager_80009548 *manager, int index);
/* Looks up the entry with the given index (behaviour inferred from its callers: fn_80010244 and
 * fn_800101F8 return the addresses 0x8 and 0x14 bytes into the entry). */
boost::shared_ptr<UnkEntry_8000D0F0> fn_8000D0F0(UnkManager_80009548 *manager, int index);
/* Returns the address of the vector 0x14 bytes into the entry with the given index (behaviour
 * inferred from its body). RainEffect passes 0 for index and subtracts the translation of the
 * entry's matrix at 0x6C from the vector. */
Fn8040Vector3 *fn_800101F8(UnkManager_80009548 *manager, int index);
}

/* The matrix of the entry with the given index (name inferred; an inline function: other code
 * calls its out-of-line copy at 0x800D4E3C). */
inline Fn8040Matrix34 *GetEntryMatrix(UnkManager_80009548 *manager, int index) {
    /* The original returns the address after the temporary is destroyed, so it relies on the
     * manager holding another reference to the entry. */
    UnkEntry_8000D0F0 *entry = fn_8000D0F0(manager, index).get();

    return &entry->field_0x6C;
}

#endif
