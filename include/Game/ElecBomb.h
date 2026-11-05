#ifndef GAME_ELECBOMB_H
#define GAME_ELECBOMB_H

#include "Game/CharInfo.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/SetAdapterState.h"
#include "Game/UnkObject_802B4254.h"
#include "Game/UnkTarget_80166AF4.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

/* ElecBombData (class name from RTTI, derived from SetAdapterState): what the owner keeps of an
 * ElecBomb between its creations. The constructor stores 0 in the state when it creates it and
 * restores the state from it otherwise, and the bomb stores 2 in it when it explodes (behaviour
 * inferred). */
class ElecBombData : public SetAdapterState {
  public:
    /* 0x04 */ s32 m_state;
};
CHECK_SIZEOF(ElecBombData, 0x8);

/* ElecBomb (class name from RTTI; the gadget's descriptor is "ELEC_BOMB"). Member names and the
 * names of the non-virtual methods are inferred from the code. */
class ElecBomb : public Gadget {
  public:
    ElecBomb(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~ElecBomb();
    virtual void UpdateBody(float dt);
    /* Draws a debug sphere of radius 150 at the character info when there is a model (behaviour
     * inferred). */
    virtual void fn_800CBC1C(float);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Reads the position and rotation from the owner (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the character info's position and y rotation, and the object 170 units
     * above it (name inferred). */
    void UpdateModelMatrix();
    /* Whether the character info nearest to the bomb is at most 150 units away; if so, it passes
     * that character info and 0x35 to fn_8020642C first (name and behaviour inferred). */
    bool IsCharacterNear();

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ GadgetModelInfo *m_model;
    /* Unclassified storage; no accesses are identified. */
    /* 0x3C */ u8 field_0x3C[4];
    /* State 0 waits for a character to come near, state 1 explodes and state 2 has exploded
     * (inferred). */
    /* 0x40 */ s32 m_state;
    /* 0x44 */ boost::shared_ptr<CharInfo> field_0x44; /* only constructed and destroyed here */
    /* 0x4C */ boost::shared_ptr<UnkTarget_80166AF4> m_target;
    /* 0x54 */ boost::shared_ptr<UnkObject_802B4254> m_object;
    /* Unclassified storage in the original 0x60-byte allocation; no accesses are identified. */
    /* 0x5C */ u8 field_0x5C[4];
};
CHECK_SIZEOF(ElecBomb, 0x60);

#endif
