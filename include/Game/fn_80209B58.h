#ifndef GAME_FN_80209B58_H
#define GAME_FN_80209B58_H

#include "Game/fn_8040_vector_types.h"
#include "layout_check.h"

class Task;

/* What the EGG_BALLOON gadget passes to fn_80209B58 (placeholder name; the other callers,
 * USA (GUPE8P) 0x80194BE4 and 0x802FAD84, are not recovered). The constructor (0x80145B58) copies
 * the zero vector lbl_805E07A0 into both vectors, stores 5 and 4 in the floats and sets the flag;
 * the meanings of the members are unknown. */
struct UnkParams_80145B58 {
    UnkParams_80145B58()
        : field_0x0(lbl_805E07A0), field_0xC(lbl_805E07A0), field_0x18(5.0f), field_0x1C(4.0f),
          field_0x20(true) {}
    /* The constructor at 0x80194C1C: copies field_0x0 and field_0xC from the two vectors, stores
     * field_0x18 and field_0x20 and a constant in field_0x1C (behaviour from its body; the
     * reference spellings are inferred). */
    UnkParams_80145B58(const Fn8040Vector3 &vector0, const Fn8040Vector3 &vector1, float arg2,
                       bool arg3);

    /* 0x00 */ Fn8040Vector3 field_0x0;
    /* 0x0C */ Fn8040Vector3 field_0xC;
    /* 0x18 */ float field_0x18;
    /* 0x1C */ float field_0x1C;
    /* 0x20 */ bool field_0x20;
};
CHECK_SIZEOF(UnkParams_80145B58, 0x24);

extern "C" {
/* Creates a 0xD4-byte object with kind (constructor 0x8020CAAC), passes params to its virtual
 * function at vtable offset 0xC and creates a 0x30-byte task under parent for it (0x80209CA8)
 * when that allocation succeeds (behaviour from its body). The object is a ThrowBombImpl (class
 * name from RTTI; see ThrowBomb.h). */
void fn_80209B58(Task *parent, int kind, const UnkParams_80145B58 *params);
}

#endif
