#ifndef GAME_ELECCIRCUITCOLLI_H
#define GAME_ELECCIRCUITCOLLI_H

#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

/* ElecCircuitColli (class name from RTTI; the gadget's descriptor is "ELEC_CIRCUITCOLLI"). Member
 * and method names are inferred. */
class ElecCircuitColli : public Gadget {
  public:
    ElecCircuitColli(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~ElecCircuitColli();
    virtual void UpdateBody(float dt);
    virtual void fn_800CBC1C();
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position and rotation into the gadget (name inferred). */
    void CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Calls fn_8015BE64 on the handle target when there is one (purpose unknown). */
    void fn_8033BC78();
    /* The handle target's callback while its probe has hits: passes each character in the hits
     * to fn_8033C470 (name inferred). */
    void OnCollision(ColliDetectArgs &args);
    /* The handle target's callback once the hits are gone: passes 0x40000 and -1 to the probe's
     * UnkSlot20 (name inferred). */
    void OnCollisionEnd(ColliDetectArgs &args);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<UnkHandleBTarget_8015BE64> m_handle;
};
CHECK_SIZEOF(ElecCircuitColli, 0x40);

#endif
