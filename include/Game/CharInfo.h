#ifndef GAME_CHARINFO_H
#define GAME_CHARINFO_H

#include "Game/fn_8040_vector_types.h"
#include "boost/weak_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

class CharCommand;

/* CharInfo (class name from RTTI): the 0x64-byte base class of the character infos of gadgets,
 * enemies, bosses and other objects (GadgetCharInfo, EnemyCharInfo, BossCharInfo, ...). The layout
 * comes from its constructor; only the position and rotation are named (names inferred). The vtable
 * pointer sits at 0x38, after the members declared before the first virtual function. */
class CharInfo {
  public:
    /* 0x00 */ Fn8040Vector3 m_position;
    /* 0x0C */ UnkRotation3 m_rotation; /* radians */
    /* ElecSecurityCode passes field_0x18 and field_0x24 to fn_804103E8 as the velocity and the
     * acceleration (roles inferred); Meteor stores its movement for the update in field_0x18 and
     * adds it to the position. */
    /* 0x18 */ Fn8040Vector3 field_0x18;
    /* 0x24 */ Fn8040Vector3 field_0x24;
    /* 0x30 */ s32 field_0x30; /* the rotation order passed to fn_8001E070 (inferred) */
    /* 0x34 */ u32 field_0x34;

    /* Stores arg0 at 0x50 (behaviour inferred). Callers pass various small constants, for example
     * 1, 5, 0x33 and 0xAC directly and 0x58 to 0x8C through the gadget character info factories,
     * which may be ids. */
    CharInfo(int arg0);
    virtual ~CharInfo();
    /* The remaining virtual functions are named after their implementations in CharInfo's vtable
     * (0x8053D540), in slot order; their purpose is unknown. fn_8007A4CC stores its argument at
     * 0x54 (callers pass -1 or forward a value; type inferred). fn_801DC980 and
     * fn_801DC97C take a command: ElecCircuitEntrance passes one to the first, and the overrides
     * of the second read the command type at 0x4 and the flags at 0x7. */
    virtual void fn_8007A4CC(int arg0);
    virtual void fn_801DC980(CharCommand *command);
    virtual void fn_801DC97C(CharCommand *command);
    /* CharInfo's implementations return 0; the overrides return 0, 1 or another character info's
     * result. The callers of all but fn_80078B3C (none found) test only the low byte of the
     * result, so bool is inferred. */
    virtual bool fn_80078B2C();
    virtual bool fn_80078B34();
    virtual bool fn_80078B3C();
    virtual bool fn_80078B44();
    virtual bool fn_80078B4C();
    virtual bool fn_80073634();

    /* Stores value in the word at 0x5C (name inferred).
     * Matching workaround: this inline member stands for an unknown original accessor. Called
     * through a smart pointer, an inline member leaves the smart pointer's operator-> out of line,
     * as the character info factories at 0x8015CB88 and 0x8015CD90 and the Researcher's factory do
     * where they store this word; a plain member store expands the operator in place. Revisit when
     * the original accessor is identified. */
    void SetField0x5C(s32 value) {
        field_0x5C = value;
    }
    /* Stores value in the word at 0x50 (name inferred).
     * Matching workaround: as SetField0x5C, this inline member stands for an unknown original
     * accessor; the Catapult gadget's command handler calls the operator-> of its character info
     * pointer out of line where it stores this word. Revisit with SetField0x5C. */
    void SetField0x50(s32 value) {
        field_0x50 = value;
    }

    /* 0x3C */ u8 field_0x3C[0x50 - 0x3C];
    /* 0x50 */ s32 field_0x50; /* the constructor's arg0 */
    /* 0x54 */ s32 field_0x54; /* set by fn_8007A4CC; PlayerBase compares it with 0 as signed */
    /* 0x58 */ u8 field_0x58[0x5C - 0x58];
    /* 0x5C */ s32 field_0x5C;         /* cleared by the constructor */
    /* 0x60 */ const void *field_0x60; /* the constructor stores a data address here */
};
CHECK_SIZEOF(CharInfo, 0x64);

/* The manager returned by fn_80074194 keeps a list of character infos. */
class UnkManager_80074194;

/* An empty shared pointer that callers pass where no character info is wanted (behaviour inferred
 * from its uses). */
extern boost::shared_ptr<CharInfo> lbl_805EE698;

/* Another default-constructed shared pointer of the same kind, set up by the start-up function
 * at 0x801DCABC (behaviour inferred from its uses). */
extern boost::shared_ptr<CharInfo> lbl_805EF980;

extern "C" {
/* Returns the manager (behaviour inferred): constructs a function-local static manager on first
 * use, registers its destructor and caches its address in a static pointer. The E3 2005
 * prototype (G9SJ8P) expands this sequence in its callers (62 copies), which suggests a header
 * inline accessor. */
UnkManager_80074194 *fn_80074194(void);
/* Returns the listed character info nearest to position, or an empty pointer when the list is
 * empty (behaviour inferred). */
boost::weak_ptr<CharInfo> fn_800A8A7C(UnkManager_80074194 *manager, const Fn8040Vector3 *position);
/* Returns the listed character info nearest to the segment (by fn_8003C660's distance), or an
 * empty pointer when the list is empty (behaviour inferred). The segment is taken by const
 * reference (inferred): the ELEC_LIGHT gadget passes a temporary whose constructor's result it
 * hands on. */
boost::weak_ptr<CharInfo> fn_800A8810(UnkManager_80074194 *manager,
                                      const UnkSegment_80020A20 &segment);
/* Returns the listed character info whose object has the word id at 0x298, or an empty pointer
 * when there is none (behaviour inferred). */
boost::weak_ptr<CharInfo> fn_800A8B74(UnkManager_80074194 *manager, s32 id);
/* Returns 0 for arg1 0x27, otherwise fn_802063BC's result (return type inferred, the gadgets
 * ignore it). */
int fn_8020642C(const boost::shared_ptr<CharInfo> &charInfo, int arg1);
/* Builds the character info's world matrix: the identity, then its position and its three
 * rotations, in the order that the rotation order at 0x30 selects; does nothing when matrix is null
 * (behaviour inferred from its body). */
void fn_8015CEA0(const CharInfo *info, Fn8040Matrix34 *matrix);
}

/* The id that callers pass to fn_800A8B74 (the meaning is unknown; 43 functions read it). */
extern s32 lbl_805EE588;

#endif
