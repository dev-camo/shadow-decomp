#ifndef GAME_CHARINFO_H
#define GAME_CHARINFO_H

#include "Game/fn_8040_vector_types.h"
#include "boost/weak_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

/* CharInfo (class name from RTTI): the 0x64-byte base class of the character infos of gadgets,
 * enemies, bosses and other objects (GadgetCharInfo, EnemyCharInfo, BossCharInfo, ...). The layout
 * comes from its constructor; only the position and rotation are named (names inferred). The vtable
 * pointer sits at 0x38, after the members declared before the first virtual function. */
class CharInfo {
  public:
    /* 0x00 */ Fn8040Vector3 m_position;
    /* 0x0C */ Fn8040Vector3 m_rotation; /* radians */
    /* 0x18 */ Fn8040Vector3 field_0x18;
    /* 0x24 */ Fn8040Vector3 field_0x24;
    /* 0x30 */ s32 field_0x30; /* the rotation order passed to fn_8001E070 (inferred) */
    /* 0x34 */ u32 field_0x34;

    virtual ~CharInfo();

    /* 0x3C */ u8 field_0x3C[0x64 - 0x3C];
};
CHECK_SIZEOF(CharInfo, 0x64);

/* The manager returned by fn_80074194 keeps a list of character infos. */
class UnkManager_80074194;

extern "C" {
/* Returns the manager (behaviour inferred): constructs a function-local static manager on first
 * use, registers its destructor and caches its address in a static pointer. The E3 2005
 * prototype (G9SJ8P) expands this sequence in its callers (62 copies), which suggests a header
 * inline accessor. */
UnkManager_80074194 *fn_80074194(void);
/* Returns the listed character info nearest to position, or an empty pointer when the list is
 * empty (behaviour inferred). */
boost::weak_ptr<CharInfo> fn_800A8A7C(UnkManager_80074194 *manager, const Fn8040Vector3 *position);
}

#endif
