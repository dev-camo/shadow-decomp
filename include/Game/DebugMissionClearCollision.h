#ifndef GAME_DEBUGMISSIONCLEARCOLLISION_H
#define GAME_DEBUGMISSIONCLEARCOLLISION_H

#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

class UnkManager_80074470;

extern "C" {
/* Returns the manager (behaviour inferred): constructs a function-local static manager on first
 * use, registers its destructor and caches its address in a static pointer, as fn_80074194 does. */
UnkManager_80074470 *fn_80074470(void);
/* Stores id in the manager's word at 0xC unless the byte at 0x17 + id is set, or the byte at 0x14
 * is set and flag is false (behaviour inferred from its body; the parameter types are inferred). */
void fn_80175EEC(UnkManager_80074470 *manager, int id, bool flag);
}

/* The gadget parameter in the order of the descriptor's table. */
struct DebugMissionClearCollisionParams {
    /* 0x0 */ u32 type; /* the parameter "0:DK 1:NM 2:HR 3:BOSS" */
};
CHECK_SIZEOF(DebugMissionClearCollisionParams, 0x4);

/* DebugMissionClearCollision (class name from RTTI; the gadget's descriptor is
 * "DebugDebugMissionClearCollision"). Method names marked "(name inferred)" are not original. */
class DebugMissionClearCollision : public Gadget {
  public:
    DebugMissionClearCollision(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~DebugMissionClearCollision();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position, rotation and parameter into the gadget (name inferred). */
    void CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Queues a debug sphere of the trigger radius and the four debug lines of an M, placed at the
     * gadget and turned about its y axis, in the colours of the type (name inferred). */
    void DrawDebugMarker();

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ DebugMissionClearCollisionParams m_params;
    /* 0x3C */ u32 field_0x3C; /* in the allocation size (0x40); no code here touches it */
};
CHECK_SIZEOF(DebugMissionClearCollision, 0x40);

#endif
