#ifndef GAME_ELECFIREWALL_H
#define GAME_ELECFIREWALL_H

#include "Game/CharaColliAttack.h"
#include "Game/CharInfo.h"
#include "Game/Gadget.h"
#include "Game/GadgetCharInfo.h"
#include "Game/GadgetModelInfo.h"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "rw/rwcore.h"
#include "layout_check.h"

/* The gadget parameters in the order of the ELEC_FIREWALL descriptor's table, whose entry names
 * give the member names. fn_800C9DBC reads them from the owner. */
struct ElecFireWallParams {
    /* 0x00 */ float scaleX;
    /* 0x04 */ float scaleY;
    /* 0x08 */ float scaleZ;
    /* 0x0C */ float searchRange;
    /* 0x10 */ float initialY;
};
CHECK_SIZEOF(ElecFireWallParams, 0x14);

/* ElecFireWall (class name from RTTI; the gadget's descriptor is "ELEC_FIREWALL"): a wall of the
 * FIREWALL.BIN model that starts at its "Initial Y" offset or 100 times its y scale below its
 * position, whichever is lower. Once the nearest character info comes within its search range,
 * it rises to 100 times its y scale below its position, jumps 3% of that depth higher and settles
 * back there. When the first callback of its attack object runs, the wall breaks up with an
 * effect and a sound and finishes (behaviour inferred). Method names marked "(name inferred)" are
 * not original. */
class ElecFireWall : public Gadget {
  public:
    ElecFireWall(Task *parent, UnkGadgetOwner_800C9CD0 *owner);
    virtual ~ElecFireWall();
    virtual void UpdateBody(float dt);
    virtual void ResetFromOwner(UnkGadgetOwner_800C9CD0 *owner);

    /* Creates the wall under task layer 11 (name inferred). */
    static void Create(UnkGadgetOwner_800C9CD0 *owner);

    /* Copies the owner's position, rotation and parameters into the wall (name inferred). */
    void CopyFromOwner(UnkGadgetOwner_800C9CD0 *owner);
    /* Places the model at the character info's position raised by m_height, turned by its yaw
     * and scaled by the parameters (name inferred). */
    void UpdateModelMatrix();
    /* Advances a position and a velocity by duration under a constant acceleration and a drag
     * coefficient, as ElecBlock::Integrate does (name inferred). */
    void Integrate(float *position, float *velocity, float acceleration, float drag,
                   float duration);
    /* True when the nearest character info is within the search range (name inferred). */
    bool IsCharInfoInRange();
    /* Passes the "Scale" parameters, multiplied by three fixed factors, to the model, and the y
     * product raised by m_height as a vertical offset (name inferred). */
    void ApplyScale();
    /* Scales the frame of the model part with the id 1 vertically by m_partScaleY on top of the
     * matrix that frame has when it is found (name inferred). */
    void UpdatePartFrame();
    /* Shows the break effect of model entry 3 and plays a sound (name inferred). */
    void ShowBreakEffect();
    /* The two callbacks the wall gives its attack object with fn_80054DD0. The first passes the
     * character info's vector at 0x18 to the attacked object as a direction and a length (the
     * unit y vector and 1 for a zero vector) and lets the wall break up in the next update
     * (behaviour inferred); the second does nothing. */
    void fn_801F6764(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);
    void fn_801F6760(SonicteamUSA::System::CharaColli::AttackCallbackParam &param);

    static const char *s_name;

    /* 0x30 */ ElecFireWallParams m_params;
    /* 0x44 */ boost::shared_ptr<GadgetCharInfo> m_charInfo;
    /* 0x4C */ GadgetModelInfo *m_model;
    /* The state (inferred from the update): 0 waiting for a character info in range, 1 rising,
     * 2 settling with Integrate, 3 at rest, 4 breaking up (set by fn_801F6764). */
    /* 0x50 */ s32 m_state;
    /* Offset above the position, set from m_params.initialY: the constructor caps it at -100
     * times the y scale, ResetFromOwner does not. */
    /* 0x54 */ float m_height;
    /* 0x58 */ float m_speed; /* rate of change of m_height (inferred) */
    /* 1 while the wall is at least 370 times its y scale below its position, otherwise m_height
     * divided by -370 times its y scale (set by UpdateBody) */
    /* 0x5C */ float m_partScaleY;
    /* 0x60 */ RwFrame *m_partFrame;  /* the frame of the model part with the id 1 */
    /* 0x64 */ RwMatrix m_partMatrix; /* a copy of that frame's matrix, taken when it is found */
    /* 0xA4 */ boost::shared_ptr<CharInfo> field_0xA4; /* only constructed and destroyed */
    /* The model's attack object, to which the constructor gives the two callbacks. Nothing else
     * uses it, but the destructor destroys a weak_ptr at this offset, so it is a member. */
    /* 0xAC */ boost::weak_ptr<SonicteamUSA::System::CharaColli::CharaColliAttack> m_attack;
    /* Unclassified storage in the original 0xB8-byte allocation; no accesses are identified. */
    /* 0xB4 */ u8 field_0xB4[4];
};
CHECK_SIZEOF(ElecFireWall, 0xB8);

#endif
