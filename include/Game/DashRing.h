#ifndef GAME_DASHRING_H
#define GAME_DASHRING_H

#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/UnkHandleBTarget_8015BE64.h"
#include "Game/UnkObject_801695B8.h"
#include "Game/UnkSoundHandle_801D3DC8.h"
#include "boost/shared_ptr.hpp"
#include "layout_check.h"

namespace DashRing {

/* The gadget parameters in the order of the descriptor's table. */
struct DashRingParams {
    DashRingParams() : level(0.0f), noControlTime(0.0f) {}

    /* 0x0 */ float level;         /* 0 to 15: selects the launch speed */
    /* 0x4 */ float noControlTime; /* seconds the character cannot be controlled */
};
CHECK_SIZEOF(DashRingParams, 0x8);

/* DashRing::DashRing (class name from RTTI; the gadget's descriptor is "DASHRING"): a ring that,
 * when a character touches the "col" node of its model, takes the character's control for a time,
 * places it at the ring and launches it along the ring's direction with a speed that the level
 * selects; it keeps a sound running at its position (behaviour inferred). Member and method names
 * are inferred. */
class DashRing : public Gadget {
  public:
    DashRing(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~DashRing();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Reads the position, rotation and parameters from the owner (name inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the character info with its rotation about the y, x and z axes (name
     * inferred). */
    void UpdateModelMatrix();
    /* The model's collision callback: passes the hits to fn_8015B004 with the hit callback and a
     * hit context (name inferred). */
    void OnCollision(ColliDetectArgs &args);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x40 */ boost::shared_ptr<UnkObject_801695B8> m_handle;
    /* 0x48 */ DashRingParams m_params;
    /* 0x50 */ UnkSoundHandle_801D3DC8 m_sound;
    /* 0x6C */ u8 field_0x6C[0x4]; /* in the allocation size (0x70); no code here touches it */
};
CHECK_SIZEOF(DashRing, 0x70);

} // namespace DashRing

#endif
