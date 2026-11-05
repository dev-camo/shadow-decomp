#ifndef GAME_SUCTIONPILLAR_H
#define GAME_SUCTIONPILLAR_H

#include "Game/CharCommand.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/Target.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/UnkObject_80169BB0.h"
#include "Game/UnkTarget_80166AF4.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

namespace SuctionPillar {

/* SuctionPillar::SuctionPillar (class name from RTTI; the gadget's descriptor is "SUCTION
 * PILLAR"): a pillar that, once its character info receives a command with id 0x14, sets its
 * stage switch (if it has one), creates an object of kind 8 at its position (fn_80169B1C) and
 * rises along its local z axis to a fixed height, playing a sound while it moves and another when
 * it stops (behaviour inferred). Method names marked "(name inferred)" are not original; all
 * member names are inferred. */
class SuctionPillar : public Gadget {
  public:
    SuctionPillar(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~SuctionPillar();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position, rotation and switch id, keeps the position as the base of the
     * rise and resets the state, the flags and the rise (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the character info's position and rotation, and the two objects at
     * fixed offsets in the model's frame (name inferred). */
    void UpdateModelMatrix();
    /* The character info's command handler (name inferred): the first command with id 0x14 sets
     * flag 0, which starts the rise. */
    void OnCommand(CharCommand &command);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x40 */ boost::shared_ptr<Target> m_objects[2];
    /* 0x50 */ boost::shared_ptr<UnkTarget_80166AF4> m_target;
    /* 0 until the first update after construction or a reset, then 1 while waiting for the
     * command and rising, and 2 once the pillar has stopped (inferred). */
    /* 0x58 */ s32 m_state;
    /* 0x5C */ Fn8040Vector3 m_basePosition; /* the owner's position, where the rise starts */
    /* 0x68 */ UnkBitFlags_80014E04 m_flags; /* bit 0: the command has been received */
    /* 0x6C */ float m_rise;                 /* the distance risen so far */
    /* 0x70 */ s32 m_switchId; /* the id of the stage switch that the command sets (inferred) */
    /* 0x74 */ boost::weak_ptr<UnkObject_80169BB0> m_commandObject; /* fn_80169B1C's object */
};
/* This layout uses 4 bytes of tail padding from Task's 8-byte alignment. */
CHECK_SIZEOF(SuctionPillar, 0x80);

} // namespace SuctionPillar

#endif
