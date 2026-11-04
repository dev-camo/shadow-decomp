#ifndef GAME_ARKCANNON_H
#define GAME_ARKCANNON_H

#include "Game/CharaColliAttack.h"
#include "Game/CharInfo.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/Target.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include <vector>
#include "layout_check.h"

/* The two words that the gadget parameters "0:GRAND 1:AIR" and "RANGE m" fill, in this order. */
struct ArkCannonParams {
    /* 0x0 */ u32 type; /* 0 ground, 1 air (from the parameter's label) */
    /* 0x4 */ float range;
};
CHECK_SIZEOF(ArkCannonParams, 0x8);

/* ArkCannon (class name from RTTI): a cannon gadget with a model for each of its two types. As far
 * as the code shows (behaviour inferred), it waits until the nearest character comes within its
 * range, turns towards it, fires four shots, waits and starts again; attacks lower its health, and
 * when the health is used up it shows its break effect (the air type shows both break models) and
 * finishes. Method names marked "(name inferred)" are not original; all member names are
 * inferred. */
class ArkCannon : public Gadget {
  public:
    ArkCannon(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~ArkCannon();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position, rotation and parameters, turns the range into game units and
     * resets the health, the timers and the shot count (name inferred). */
    void InitFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Selects the model of the type and registers OnAttack with its attack object (name
     * inferred). */
    void SetupModel();
    /* Places the model and the objects at the character info's position and rotation (name
     * inferred). */
    void UpdateModelMatrix();
    /* True when the nearest character is closer than the range (name inferred). */
    bool IsTargetInRange();
    /* Turns the cannon towards the target by at most 45 degrees a second and returns whether the
     * angle left was smaller than this frame's turn (name inferred). */
    bool TurnToTarget(boost::shared_ptr<CharInfo> target, float dt);
    /* Fires shot number index at the target from the model frame that s_shotFrames gives (name
     * inferred). */
    void Fire(u32 index, boost::shared_ptr<CharInfo> target);
    /* Enters state and does its set-up (name inferred). */
    void SetState(int state);
    /* The attack object's callback (name inferred). */
    void OnAttack(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);

    static const char *s_name;

    /* 0x30 */ ArkCannonParams m_params;
    /* 0x38 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x40 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x48 */ boost::shared_ptr<Target> m_object;
    /* 0x50 */ std::vector<boost::shared_ptr<Target> > m_objects;
    /* 0x5C */ boost::weak_ptr<CharInfo> m_target; /* the character being aimed at */
    /* 0 waiting, 1 turning, 2 firing, 3 reloading, 4 destroyed (inferred from SetState and
     * UpdateBody). */
    /* 0x64 */ int m_state;
    /* Bit 0 is set by every attack, bit 1 when the health is used up (inferred). */
    /* 0x68 */ UnkBitFlags_80014E04 m_attackFlags;
    /* 0x6C */ float m_health;
    /* 0x70 */ float m_reloadTimer;
    /* 0x74 */ float m_shotTimer;
    /* 0x78 */ u32 m_shotCount;
    /* 0x7C */ u32 field_0x7C; /* not accessed here; the object is 0x80 bytes */
};
CHECK_SIZEOF(ArkCannon, 0x80);

#endif
