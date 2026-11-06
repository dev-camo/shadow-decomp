#ifndef GAME_SETCOLLISION_H
#define GAME_SETCOLLISION_H

#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

namespace SetCollision {

/* The gadget parameters in the order of the descriptor's table (names from the table's strings
 * and value labels). The constructor stores the defaults the gadget starts with. */
struct SetCollisionParams {
    SetCollisionParams() : kind(0), form(0), x(1.0f), y(1.0f), z(1.0f), id(0), appear(1) {}

    /* "kind": 0 "Guard", 1 "Not Available", 2 "Start", 3 "Damage", 4 "Dead", 5
     * "ChaosControlCancelOn", 6 "ChaosControlCancelOff", 7 "ChaosControlStop", 8 "FlyEnd", 9
     * "LaunchContinue", 10 "NoInput", 11 "Goal", 12 "Off" */
    /* 0x00 */ s32 kind;
    /* 0x04 */ s32 form; /* 0 "Sphere", 1 "Box", 2 "Cylinder" */
    /* 0x08 */ float x;  /* labelled "Radius" for a sphere or cylinder, "Width" for a box */
    /* 0x0C */ float y;  /* labelled "Height" for a box, empty otherwise */
    /* 0x10 */ float z;  /* labelled "Depth" for a box, "Height" for a cylinder, else empty */
    /* "Id": the entry of fn_800CB69C's table that UpdateBody reads and the "Start" and "Off"
     * callbacks set */
    /* 0x14 */ s32 id;
    /* 0x18 */ s32 appear; /* 0 "Disappear", 1 "Appear", 2 "No Use" */
};
CHECK_SIZEOF(SetCollisionParams, 0x1C);

/* SetCollision::SetCollision (class name from RTTI; the gadget's descriptor is "SETCOLLISION"): a
 * probe without a model, a sphere, box or cylinder by the parameter "form", sized by "X", "Y" and
 * "Z". The constructor copies the owner's parameter block (fn_800C9DBC, only when its size is that
 * of SetCollisionParams) before it creates the probe. "kind" selects the flags word and the integer
 * after it in the probe factory's arguments (0x102 and 7 for kind 0, 0x104 and 7 for 3, 0x120 and 7
 * for 4, 0x40100 and 10 for 1, 2 and 5-12, 0 and 7 for any other value), the value stored at 0x50
 * of the character info (0x6F-0x74 for kinds 5-10, 0 otherwise) and, for kinds 1, 2, 11 and 12,
 * the collision callback bound to the handle target; the other kinds bind none. For a "form"
 * outside 0-2 no probe is created, and the handle target is given an empty probe pointer, which
 * CreateCollision's SetSize call still uses.
 *
 * Each update first runs the handle target (fn_8015BE9C), when there is one, whose callback can
 * move the gadget from state 2 to state 3 before the state is read; then: state 0 becomes state 2
 * for "appear" 0 and state 1 otherwise; in state 1, once fn_8016904C reports 1 for "Id", it
 * becomes state 2 and, when there is a handle target, calls the probe's UnkSlot1(-1); in state 2
 * with "appear" 0 it becomes state 3 once fn_8016904C reports 1 for "Id"; in state 3 it passes
 * itself to fn_8015A794 and sets Task flag 0x01. With "appear" 1 the constructor calls the probe's
 * UnkSlot2(-1) when there is a handle target. The "Start", "Goal" and "Off" callbacks act only in
 * state 2 and only when fn_8015D3E8 reports true for the hits (one of them has a character info
 * that still exists and whose fn_80078B2C reports true), then move the gadget to state 3; the
 * "Not Available" callback does nothing. Behaviour inferred; method names marked "(name
 * inferred)" are not original. */
class SetCollision : public Gadget {
  public:
    SetCollision(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~SetCollision();
    virtual void UpdateBody(float dt);
    /* Calls fn_8015BE64 on the handle target when there is one (purpose unknown). */
    virtual void fn_800CBC1C(float);
    /* Copies the owner's settings (CopyFromOwner) and, when "form" or "kind" changed, releases
     * the handle target and creates the probe again; then passes "X", -2, "Y" and "Z" to the
     * probe's SetSize when there is a handle target. */
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position and rotation into the character info, when there is one, and
     * the parameters when the owner's parameter block has their size (fn_800C9DBC) (name
     * inferred). */
    void CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Creates the probe of the parameter "form" with the flags of "kind", stores a value selected
     * by "kind" (0x6F-0x74 for kinds 5-10, 0 otherwise) at 0x50 of the character info when there
     * is one, wraps the probe in a new handle target, binds the callback of "kind" for kinds 1, 2,
     * 11 and 12 only and passes "X", -2, "Y" and "Z" to the probe's SetSize (name inferred). */
    void CreateCollision();
    /* The handle target's callback for kind 1 ("Not Available"); it does nothing (name
     * inferred). */
    void OnCollisionNotAvailable(ColliDetectArgs &args);
    /* The callback for kind 2 ("Start"): in state 2, when fn_8015D3E8 reports true for the hits,
     * sets the state of entry "Id" of fn_800CB69C's table to 1 and moves the gadget to state 3
     * (name inferred). */
    void OnCollisionStart(ColliDetectArgs &args);
    /* The callback for kind 11 ("Goal"): in state 2, when fn_8015D3E8 reports true for the hits,
     * passes 6 and false to fn_80175EEC with fn_80074470's manager and moves the gadget to state 3
     * (name inferred). */
    void OnCollisionGoal(ColliDetectArgs &args);
    /* The callback for kind 12 ("Off"): in state 2, when fn_8015D3E8 reports true for the hits,
     * sets the state of entry "Id" of fn_800CB69C's table to 0 and moves the gadget to state 3
     * (name inferred). */
    void OnCollisionOff(ColliDetectArgs &args);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<UnkHandleBTarget_8015BE64> m_handle; /* of the probe */
    /* 0x40 */ SetCollisionParams m_params;
    /* 0 after construction, 1 waiting for "Id", 2 active, 3 finishing (inferred). */
    /* 0x5C */ s32 m_state;
};
CHECK_SIZEOF(SetCollision, 0x60);

} // namespace SetCollision

#endif
