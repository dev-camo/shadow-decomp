#ifndef GAME_ELECCOREWALL_H
#define GAME_ELECCOREWALL_H

#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "Game/UnkLayer_8004DFB0.h"
#include "boost/shared_ptr.hpp"
#include <dolphin/types.h>
#include "layout_check.h"

struct RpClump;

/* ElecCoreWall (class name from RTTI; the gadget's descriptor is "ELEC_COREWALL"). Member names and
 * the names of the non-virtual methods are inferred from the code. */
class ElecCoreWall : public Gadget {
  public:
    ElecCoreWall(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~ElecCoreWall();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position and rotation into the gadget (name inferred). */
    void CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* The draw layer's callback: when the sphere of radius 1000 around the gadget is in view, draws
     * copies of the clumps turned about the y axis in steps of a 32nd of a turn: the 11 and 21
     * clumps with every delta morph target weight set to the timer while it is below 0.76666665,
     * the 12 and 22 clumps from then on, and nothing while the timer is negative (name
     * inferred). */
    void OnDraw(RednerDrawArgs &args);
    /* The handle target's callback while its probe has hits: sets the state to STATE_FORWARD when
     * a character info is among the hits (name inferred). */
    void OnCollision(ColliDetectArgs &args);
    /* The handle target's callback once the hits are gone: sets the state to STATE_BACKWARD
     * (name inferred). */
    void OnCollisionEnd(ColliDetectArgs &args);

    /* The values of m_state (names inferred). */
    enum State { STATE_IDLE, STATE_FORWARD, STATE_BACKWARD, STATE_DONE };

    static const char *s_name;

    /* STATE_IDLE at rest, STATE_FORWARD while the timer runs forwards, STATE_BACKWARD while it runs
     * backwards and STATE_DONE once OnDraw has drawn the end of the animation in STATE_FORWARD. */
    /* 0x30 */ State m_state;
    /* 0x34 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x3C */ boost::shared_ptr<UnkHandleBTarget_8015BE64> m_handle;
    /* 0x44 */ boost::shared_ptr<UnkLayer_8004DFB0> m_layer;
    /* The clumps of DE0403COREWALL11.DFF, 21.DFF, 12.DFF and 22.DFF. */
    /* 0x4C */ RpClump *m_clump11;
    /* 0x50 */ RpClump *m_clump21;
    /* 0x54 */ RpClump *m_clump12;
    /* 0x58 */ RpClump *m_clump22;
    /* Advanced by dt in STATE_FORWARD and wound back in STATE_BACKWARD; OnDraw uses it as the delta
     * morph target weight of m_clump11 and m_clump21. */
    /* 0x5C */ float m_timer;
    /* Only cleared by the constructor; no other accesses are identified. */
    /* 0x60 */ u32 field_0x60;
    /* 0x64 */ u32 field_0x64;
};
CHECK_SIZEOF(ElecCoreWall, 0x68);

#endif
