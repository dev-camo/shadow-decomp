#ifndef GAME_LONGSPRING_H
#define GAME_LONGSPRING_H

#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/Target.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "Game/UnkObject_801695B8.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

/* The gadget parameters in the order of the descriptor's table. */
struct LongSpringParams {
    LongSpringParams() : level(0.0f), noControlTime(0.0f) {}

    /* 0x0 */ float level;         /* selects the launch speed */
    /* 0x4 */ float noControlTime; /* seconds the character cannot be controlled */
};
CHECK_SIZEOF(LongSpringParams, 0x8);

namespace LongSpring {

/* LongSpring::LongSpring (class name from RTTI; the gadget's descriptor is "LONGSPRING"): a spring
 * with three launch points across its width. A character that touches its "col" node is placed at
 * the nearest of the three points and launched along the spring's up direction with a speed that
 * the gadget's level selects (behaviour inferred). Member and method names are inferred. */
class LongSpring : public Gadget {
  public:
    LongSpring(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~LongSpring();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Reads position, rotation and parameters from the owner (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the character info's position with its rotation, and the three launch
     * points and their Targets at their offsets in the model's frame (name inferred). */
    void UpdateModelMatrix();
    /* The model's collision callback (name inferred). */
    void OnCollision(ColliDetectArgs &args);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x40 */ boost::shared_ptr<UnkObject_801695B8> m_handle;
    /* 0x48 */ LongSpringParams m_params;
    /* 0x50 */ Fn8040Vector3 m_launchPoints[3];
    /* 0x74 */ boost::shared_ptr<Target> m_targets[3]; /* moved with the launch points */
    /* Unclassified storage in the original 0x90-byte allocation; no accesses are identified. */
    /* 0x8C */ unsigned char field_0x8C[4];
};
CHECK_SIZEOF(LongSpring, 0x90);

} // namespace LongSpring

#endif
