#ifndef GAME_MONSTERCONTROL_H
#define GAME_MONSTERCONTROL_H

#include "Game/Gadget.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

/* The gadget parameters in the order of the descriptor's table. ReadOwner scales the three lengths
 * by 10 after reading them. */
struct MonsterControlParams {
    /* 0x0 */ s32 kind;      /* "0:START 1:MISS 2:END"; passed on to the monster */
    /* 0x4 */ float xLength; /* X_LENGTH m */
    /* 0x8 */ float yLength; /* Y_LENGTH m */
    /* 0xC */ float zLength; /* Z_LENGTH m */
};
CHECK_SIZEOF(MonsterControlParams, 0x10);

/* MonsterControl (class name from RTTI; the gadget's descriptor has the same name): a box probe
 * sized by the three length parameters. When fn_8015D2EC reports a hit in the probe's results, the gadget passes its kind parameter to
 * the Monster gadget's character info, which then calls DisableCollision (behaviour inferred).
 * Method names marked "(name inferred)" are not original. */
class MonsterControl : public Gadget {
  public:
    MonsterControl(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~MonsterControl();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Reads parameters, position and rotation from the owner (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Gives the probe the gadget's position, rotation and lengths (name inferred). */
    void UpdateProbe();
    /* The probe's collision callback (name inferred). */
    void OnCollision(ColliDetectArgs &args);
    /* Sets bit 1 of the handle's flags, so that fn_8015BE9C no longer processes the probe's hits
     * (name inferred). MonsterCharInfo calls it after passing the kind on. */
    void DisableCollision();

    static const char *s_name;

    /* 0x30 */ Fn8040Vector3 m_position;
    /* 0x3C */ UnkRotation3 m_rotation;
    /* 0x48 */ MonsterControlParams m_params;
    /* 0x58 */ boost::shared_ptr<UnkHandleBTarget_8015BE64> m_handle;
};
CHECK_SIZEOF(MonsterControl, 0x60);

#endif
