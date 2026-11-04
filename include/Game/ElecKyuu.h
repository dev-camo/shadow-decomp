#ifndef GAME_ELECKYUU_H
#define GAME_ELECKYUU_H

#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "Game/UnkLayer_8004DFB0.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

struct RpClump;

/* ElecKyuu (class name from RTTI; the gadget's descriptor is "ELEC_KYUU"). Member and method names
 * are inferred. */
class ElecKyuu : public Gadget {
  public:
    ElecKyuu(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~ElecKyuu();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position and rotation into the gadget (name inferred). */
    void CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Turns the character info about its y axis by a third of a turn per unit of dt (name
     * inferred). */
    void Rotate(float dt);
    /* The draw layer's callback: when the sphere of radius 302.5 around the gadget is in view,
     * draws the clump 16 times at the gadget's position, turned about the y axis in steps of a
     * sixteenth of a turn from the gadget's y rotation (name inferred). */
    void OnDraw(RednerDrawArgs &args);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<UnkLayer_8004DFB0> m_layer;
    /* 0x38 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x40 */ boost::shared_ptr<UnkHandleBTarget_8015BE64> m_handle;
    /* 0x48 */ RpClump *m_clump;
    /* Unclassified storage in the original 0x50-byte allocation; no accesses are identified. */
    /* 0x4C */ unsigned char field_0x4C[4];
};
CHECK_SIZEOF(ElecKyuu, 0x50);

#endif
