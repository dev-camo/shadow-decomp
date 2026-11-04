#ifndef GAME_FALLROAD_H
#define GAME_FALLROAD_H

#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/UnkProbeTarget_800587A0.h"
#include "Game/UnkTarget_80166AF4.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

/* FallRoad (class name from RTTI): the gadget type FALL_LARGE. Member and method names are
 * inferred (the virtual methods use Gadget's inferred names). */
class FallRoad : public Gadget {
  public:
    FallRoad(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~FallRoad();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Allocates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position and rotation (keeping only the rotation about the y axis), the
     * height parameter and the owner's byte (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the falling position, turned about the y axis and raised by the bounce
     * offset (name inferred). */
    void UpdateModelMatrix();

    static const char *s_name;

    /* 0x30 */ float m_height; /* the parameter "Height": how far above the owner the road starts */
    /* 0x34 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x3C */ GadgetModelInfo *m_model;
    /* 0x40 */ s32 m_state;              /* 0 waiting, 1 motion, 2 falling, 3 landed (inferred) */
    /* 0x44 */ u8 m_tag;                 /* copied from the owner with fn_800C9E60 */
    /* 0x48 */ Fn8040Vector3 m_position; /* the falling position */
    /* 0x54 */ Fn8040Vector3 m_velocity;
    /* 0x60 */ boost::shared_ptr<UnkProbeTarget_800587A0> m_probe;
    /* 0x68 */ boost::shared_ptr<UnkTarget_80166AF4> m_target; /* created on landing */
    /* 0x70 */ float m_bounce;     /* the model's vertical offset after landing */
    /* 0x74 */ float m_prevBounce; /* m_bounce of the previous frame */
};
CHECK_SIZEOF(FallRoad, 0x78);

#endif
