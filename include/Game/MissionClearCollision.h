#ifndef GAME_MISSIONCLEARCOLLISION_H
#define GAME_MISSIONCLEARCOLLISION_H

#include "Game/CharInfo.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "layout_check.h"

class UnkObject_802B4254;

extern "C" {
/* Returns the listed character info whose object (the pointer at 0x0 of the character info) has
 * the word id at 0x54, or an empty pointer when there is none (behaviour inferred from its body;
 * the mission clear gadgets pass 0). */
boost::weak_ptr<CharInfo> fn_800A8C54(UnkManager_80074194 *manager, s32 id);
}

/* The gadget parameters in the order of the descriptor's table. */
struct MissionClearCollisionParams {
    /* 0x0 */ u32 type;     /* the parameter "0:DK 1:NM 2:HR" */
    /* 0x4 */ float radius; /* the parameter "COLLI_RADIUS point" */
};
CHECK_SIZEOF(MissionClearCollisionParams, 0x8);

/* MissionClearCollision (class name from RTTI; the gadget's descriptor has the same name). Method
 * names marked "(name inferred)" are not original. */
class MissionClearCollision : public Gadget {
  public:
    MissionClearCollision(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~MissionClearCollision();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position, rotation and parameters into the gadget (name inferred). */
    void CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Queues a debug sphere of the radius in the colour of the type (name inferred). */
    void DrawDebugRange();

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ MissionClearCollisionParams m_params;
    /* 0x40 */ boost::shared_ptr<UnkObject_802B4254> m_object; /* not created for type 1 */
};
CHECK_SIZEOF(MissionClearCollision, 0x48);

#endif
