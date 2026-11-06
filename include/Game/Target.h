#ifndef GAME_TARGET_H
#define GAME_TARGET_H

#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

class CharInfo;

/* Target (class name from the RTTI record at 0x805E8D78, with no bases): the 0x38-byte object that
 * fn_801E72DC creates. Gadgets keep it in a shared_ptr and move it with their position. Its
 * constructors (0x801E7490, 0x801E752C) set the vtable at 0x8053DBA8 and initialise the position,
 * the floats at 0x14-0x24, the character pointer and the flag word; only the position, the float
 * at 0x20, the character pointer and the flag word are mapped. */
class Target {
  public:
    /* The vtable slots in their original order: 0x801E73EC, then the destructor 0x801E7434.
     * fn_801E73EC returns a copy of the position (behaviour inferred from its body; name
     * unknown). */
    virtual Fn8040Vector3 fn_801E73EC();
    virtual ~Target();

    /* Matching workaround: these inline members stand for an unknown original accessor. Called as
     * a statement, an inline member leaves the smart pointer's operator-> out of line, as the
     * original does (its copy is at 0x8006092C; most of its callers read the word at 0x30 through
     * it); a plain field access expands the operator in place. TestFlags is the same stand-in for
     * a test used as the whole condition of an if. Other callers in the game set, clear and test
     * several bits of this word (inferred), so the members take a mask. Revisit when the original
     * accessor is identified. Names inferred from what they do. */
    void SetFlags(u32 mask) {
        m_flags |= mask;
    }
    void ClearFlags(u32 mask) {
        m_flags &= ~mask;
    }
    bool TestFlags(u32 mask) const {
        return (m_flags & mask) != 0;
    }
    /* Matching workaround: the same kind of stand-in for the float at 0x20, which SuctionPillar's
     * constructor and the code at 0x80104E20 store through an out-of-line operator->; a direct
     * store to the member expands the operator in place and changes SuctionPillar's constructor.
     * Revisit with the accessors above. Name inferred from what it does. */
    void SetField20(float value) {
        field_0x20 = value;
    }

    /* 0x04 */ u8 field_0x4[0x8 - 0x4];          /* Unrecovered. */
    /* 0x08 */ Fn8040Vector3 m_position;         /* fn_800CCE38 sets it (name inferred) */
    /* 0x14 */ u8 field_0x14[0x20 - 0x14];       /* Unrecovered (initialised floats). */
    /* 0x20 */ float field_0x20;                 /* -1.0f from the constructors (meaning unknown) */
    /* 0x24 */ u8 field_0x24[0x28 - 0x24];       /* Unrecovered (an initialised float). */
    /* 0x28 */ boost::weak_ptr<CharInfo> m_info; /* Set by 0x801E7490 (name inferred). */
    /* 0x30 */ u32 m_flags;                      /* Bit flags (name inferred). */
    /* 0x34 */ u8 field_0x34[0x38 - 0x34];       /* Unrecovered. */
};
CHECK_SIZEOF(Target, 0x38);
CHECK_OFFSETOF(Target, m_position, 0x8);
CHECK_OFFSETOF(Target, field_0x20, 0x20);
CHECK_OFFSETOF(Target, m_info, 0x28);
CHECK_OFFSETOF(Target, m_flags, 0x30);

extern "C" {
/* Creates the object (the meaning of the argument is unknown; the Chao and Cream gadgets pass
 * 0x2000, CircusBall and Spring 0x400 and PowerDeviceNaked 0x2800). */
boost::shared_ptr<Target> fn_801E72DC(int arg0);
/* Copies position into the object's position at 0x8 (behaviour inferred from its body). */
void fn_800CCE38(Target *object, const Fn8040Vector3 *position);
/* Hands the shared pointer on to fn_8008A1B0 (purpose unknown; CircusBall and Spring call it
 * from their destructors). */
void fn_801E7108(boost::shared_ptr<Target> &object);
/* Creates an object for the character info (the number is not understood; Coaster passes 0x2000,
 * ArkCannon 0xA400 and 0x8800, and BombingWall 0xA000). */
boost::shared_ptr<Target> fn_801E7128(boost::weak_ptr<CharInfo> info, int arg1);
/* Copies vector to the object's 0x14 (with the vector's assignment operator, 0x800091D0) and sets
 * bit 1 of m_flags (behaviour from its body; the meaning of both is unknown). Passing vector as a
 * pointer is inferred: the bytes pass its address. */
void fn_801E73B0(Target *object, const Fn8040Vector3 *vector);
}

#endif
