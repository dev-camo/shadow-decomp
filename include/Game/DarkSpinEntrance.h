#ifndef GAME_DARKSPINENTRANCE_H
#define GAME_DARKSPINENTRANCE_H

#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

/* DarkSpinEntrance (class name from RTTI; the gadget's descriptor is "DARKSPINENTRANCE"). It
 * creates a probe for its character info and passes the gadget's "radius" parameter to fn_800C7A84
 * and to the probe's SetSize. Method names marked "(name inferred)" are not original. */
class DarkSpinEntrance : public Gadget {
  public:
    DarkSpinEntrance(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~DarkSpinEntrance();
    virtual void UpdateBody(float dt);
    virtual void fn_800CBC1C(float);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position and rotation into the character info and reads the radius
     * parameter (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Passes the radius, -2, 0 and 2.5 to the probe's SetSize (name inferred). */
    void UpdateProbe();
    /* Calls fn_8015BE64 on the handle target, then passes fn_802E0D6C the character info's
     * position plus the vector (0, 0, -radius / 2) turned by the character info's rotation, that
     * rotation, 2, half the radius and 1.25 (purpose unknown). */
    void fn_80258910();

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<UnkHandleBTarget_8015BE64> m_handle;
    /* 0x40 */ float m_radius; /* the "radius" parameter */
    /* Unclassified storage in the original 0x48-byte allocation; no accesses are identified. */
    /* 0x44 */ unsigned char field_0x44[4];
};
CHECK_SIZEOF(DarkSpinEntrance, 0x48);

#endif
