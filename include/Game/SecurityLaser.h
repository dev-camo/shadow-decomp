#ifndef GAME_SECURITYLASER_H
#define GAME_SECURITYLASER_H

#include "Game/CharaColliAttack.h"
#include "Game/CharaColliBase.h"
#include "Game/EffectElement.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "Game/UnkBitFlags_80014E04.h"
#include "Game/fn_8040_vector_types.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "rw/rwcore.h"
#include "layout_check.h"

class UnkResource_80012700;

namespace SecurityLaser {

/* SecurityLaser::SetSecurityLaser (class name from RTTI; the gadget's descriptor is "SECURITY
 * LASER"): a laser that shows three entries of SECURITYLASER.BIN, the base (the "model" parameter),
 * the beam (entry 2) and the beam's end (entry 3). The beam starts at frame 1 of the base and points
 * along the gadget's rotation, turned about the y axis by a swing angle. The gadget moves back and
 * forth between its position plus and minus the "X", "Y", "Z" offset (a sine with the period
 * "move sec") and swings by up to half of "rot Y(deg)" either way (a cosine with the period "rot
 * time(sec)"). An attack object follows the beam. Type "Normal" shortens the beam at an
 * object along it; types "Normal" and "Fixed w/Eff" show an effect at the beam's end and play the
 * base model's loop sound there (behaviour inferred). Member names and the names of the
 * non-virtual methods are inferred from the code. */
class SetSecurityLaser : public Gadget {
  public:
    SetSecurityLaser(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~SetSecurityLaser();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the gadget under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Reads the position, rotation and parameters from the owner, restarts the movement and the
     * swing from the read position, forgets the beam's frame and sets the flags for the type (name
     * inferred). */
    void ReadOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the base at the character info and updates the beam; once the beam's frame is known,
     * places the beam model, scaled to the beam's length, and the end model, and plays the loop
     * sound at the beam's end while flag bit 1 is set. Until then it looks the frame up (name
     * inferred). */
    void UpdateModelMatrices();
    /* Advances the movement and the swing by dt (name inferred). */
    void UpdateMotion(float dt);
    /* Sets the beam's segment from the frame along the gadget's rotation and the swing angle, cuts
     * it at the object that fn_80021E24 finds while flag bit 0 is set, passes it to the
     * attack object (fn_8005AFE4) and moves the effect to the beam's end (name inferred). */
    void UpdateBeam();
    /* Looks up the record named "<base model name>_lp" in the model file's resource and keeps its
     * sound id (name inferred). */
    void FindLoopSound();
    /* The two callbacks the attack object is created with, in the order of fn_8005A300's
     * callback0 and callback1. Both are empty (names inferred). */
    void OnAttackCallback0(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    void OnAttackCallback1(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);

    static const char *s_name;

    /* 0x30 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x38 */ boost::shared_ptr<GadgetModelInfo> m_model;
    /* 0x40 */ boost::shared_ptr<GadgetModelInfo> m_beamModel;
    /* 0x48 */ boost::shared_ptr<GadgetModelInfo> m_endModel;
    /* 0x50 */ UnkResource_80012700 *m_resource; /* the model file's resource */
    /* The parameter block, which fn_800C9DBC reads from the owner: the entries of the descriptor's
     * table in order (names from the table's strings; the effects are inferred from the code). */
    /* 0x54 */ s32 m_modelIndex; /* "model": the model entry of the base */
    /* 0x58 */ float m_length;   /* "length": the length of the beam */
    /* 0x5C */ float m_rotTime;  /* "rot time(sec)": the period of the swing */
    /* 0x60 */ float m_rotY;     /* "rot Y(deg)": the full angle of the swing about the y axis */
    /* 0x64 */ float m_moveTime; /* "move sec": the period of the movement */
    /* "X", "Y" and "Z": the offset of the movement's turning points from the position read. */
    /* 0x68 */ float m_moveX;
    /* 0x6C */ float m_moveY;
    /* 0x70 */ float m_moveZ;
    /* 0x74 */ s32 m_type;             /* "type": 0 "Normal", 1 "Fixed", 2 "Fixed w/Eff" */
    /* 0x78 */ s32 m_loadedModelIndex; /* the model entry the base was set up with */
    /* 0x7C */ float m_moveTimer;
    /* 0x80 */ float m_moveSpeed;            /* degrees of the movement's cycle per second */
    /* 0x84 */ Fn8040Vector3 m_basePosition; /* the position read from the owner */
    /* The collision object of the base's model entry (fn_80161614), which the beam's probe skips. */
    /* 0x90 */ boost::weak_ptr<SonicteamUSA::System::CharaColli::CharaColliBase> m_colli;
    /* 0x98 */ RwFrame *m_beamFrame; /* frame 1 of the base, the start of the beam */
    /* 0x9C */ float m_rotTimer;
    /* 0xA0 */ float m_rotSpeed; /* degrees of the swing's cycle per second */
    /* 0xA4 */ float m_angle;    /* the swing angle about the y axis, in radians */
    /* 0xA8 */ float m_beamLength;
    /* 0xAC */ u16 m_loopSoundId; /* -1 (0xFFFF) when there is none */
    /* 0xB0 */ boost::shared_ptr<SonicteamUSA::System::CharaColli::CharaColliAttack> m_attack;
    /* 0xB8 */ boost::weak_ptr<Effect::System::EffectElement> m_effect;
    /* Bit 0: the beam stops at an object along it (type 0); bit 1: the effect and the loop
     * sound (types 0 and 2) (inferred). Hides Task::m_flags. */
    /* 0xC0 */ UnkBitFlags_80014E04 m_flags;
    /* 0xC4 */ u8 field_0xC4[0x4]; /* in the allocation size (0xC8); no code here touches it */
};
CHECK_SIZEOF(SetSecurityLaser, 0xC8);

} // namespace SecurityLaser

#endif
